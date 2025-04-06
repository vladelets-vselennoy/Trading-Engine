// deribit_client.cpp
//[TODO: add message parsing  try using name or ny entity returned in json object similar name , queue for write operations , error logging ]
#include "deribit_client.hpp"
#include <boost/asio/connect.hpp>
#include <iostream>
std::queue<std::string> DeribitClient::receive_queue;
std::queue<std::string> DeribitClient::send_queue;
std::mutex DeribitClient::receive_queue_mutex;
std::mutex DeribitClient::send_queue_mutex;
std::shared_ptr<spdlog::logger> DeribitClient::logger_ = nullptr;


DeribitClient::DeribitClient(const std::string& host, const std::string& port, const std::string& target)
    : host_(host),
      port_(port),
      target_(target),
      ctx_(ssl::context::tlsv12_client),
      ws_(ioc_, ctx_),
       timer_(ioc_) {
    ctx_.set_default_verify_paths();
    
    
    init_logger(); 
    
    //  Initialize logger 
    spdlog::info(" DeribitClient created for {}:{}{}", host, port, target);
}

void DeribitClient::connect() {
    tcp::resolver resolver(ioc_);
    auto const results = resolver.resolve(host_, port_);
    boost::asio::connect(beast::get_lowest_layer(ws_), results.begin(), results.end());
    ws_.next_layer().handshake(ssl::stream_base::client);
    ws_.handshake(host_, target_);
    // AFter connecting
    try {
        spdlog::info("Starting receive_thread...");
        start_receiving();
    } catch (const std::exception& e) {
        spdlog::error(" Exception while starting receive_thread: {}", e.what());
    } catch (...) {
        spdlog::error(" Unknown exception while starting receive_thread");
    }
    try{
        spdlog::info(" Starting process_thread...");
        start_message_processing();
    } catch (const std::exception& e) {
        spdlog::error(" Exception while starting process_thread: {}", e.what());
    } catch (...) {
        spdlog::error(" Unknown exception while starting process_thread");
    }
    try {
        spdlog::info(" Starting send_thread...");
        start_sending();
    } catch (const std::exception& e) {
        spdlog::error(" Exception while starting send_thread: {}", e.what());
    } catch (...) {
        spdlog::error(" Unknown exception while starting send_thread");
    }
        std::cout << " Connected to Deribit WebSocket\n";
}

void DeribitClient::start_sending(){
    send_thread_ = std::thread(&DeribitClient::send_messages, this);
}
void DeribitClient::start_message_processing() {
    process_thread_ = std::thread(&DeribitClient::process_messages, this);
}
void DeribitClient::start_receiving() {
    recv_thread_ = std::thread(&DeribitClient::receive_loop, this);
    
}
std::string DeribitClient::generate_id(const std::string& function_name) {
    return function_name + "#" + std::to_string(++msg_id_);
}


void DeribitClient::init_logger() {
    try {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/deribit.log", 20 * 1024 * 1024, 3);
        file_sink->set_level(spdlog::level::debug); // Log everything to file

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::err); // Only log errors to console

        std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};

        logger_ = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger_);
        spdlog::set_level(spdlog::level::debug);  // This sets the global filtering level
        spdlog::flush_on(spdlog::level::err);    // Flush immediately on error
        spdlog::info(" Logger initialized successfully.");
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << " Logger initialization failed: " << ex.what() << std::endl;
    }
}


void DeribitClient::handle_error(const std::string& context, const std::exception& e) {
    std::string err_msg = fmt::format(" [{}] Error: {}", context, e.what());
    spdlog::error(err_msg);
   
}

void DeribitClient::receive_loop() {
    try {
        boost::beast::flat_buffer buffer;
        while (true) {
            ws_.read(buffer);
            std::string message = boost::beast::buffers_to_string(buffer.data());
            {
                // Lock the receive queue to safely push the message
                std::lock_guard<std::mutex> lock(receive_queue_mutex);
                receive_queue.push(message);
                // std::cout << "📩 Received: " << message << "\n";
                spdlog::info("📩 Received: {}", message);
            }
            buffer.clear();
        }
    } catch (const std::exception& e) {
        handle_error("Receive Loop", e);
    }
}

void DeribitClient::process_messages() {
    while (true) {
        std::string message ;
        {
            std::lock_guard<std::mutex> lock(receive_queue_mutex);
        
        if (!receive_queue.empty()) {
            
             message = receive_queue.front();
            receive_queue.pop();
            
            
        }}
        if (!message.empty()) {
            spdlog::info("Processing received message: {}", message);
            try {
              
                parser_.parse_and_print(message);
               
            } catch (const std::exception& e) {
                handle_error("Message Processing", e);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}


void DeribitClient::send_message(const json& msg) {
    std::string message = msg.dump();
    {
        std::lock_guard<std::mutex> lock(send_queue_mutex);
        send_queue.push(message);

        spdlog::info("➡️ Added to send queue: {}", message);
    }
}


void DeribitClient::send_messages() {
    while (true) {
       
         std::lock_guard<std::mutex> lock(send_queue_mutex);
        if (!send_queue.empty()) {
             std::cout << " Waiting to send messages...\n";
           
            std::string message = send_queue.front();
            send_queue.pop();
            // Send the message via WebSocket
            ws_.write(boost::asio::buffer(message));
            // std::cout << "➡️ Sent: " << message << "\n";
            spdlog::info("➡️ Sent: {}", message);
        }
    }
}

void DeribitClient::close() {
    
    try{
    std::lock_guard<std::mutex> lock(send_queue_mutex);
    ws_.close(websocket::close_code::normal);
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
      if (send_thread_.joinable()) send_thread_.join(); 
    if (process_thread_.joinable()) process_thread_.join();
    // std::cout << "Disconnected.\n";
    spdlog::info("Disconnected from Deribit WebSocket");
    }
    catch (const std::exception& e) {
        handle_error("Close Connection", e);
    }
    catch (...) {
        spdlog::error(" Unknown exception while closing connection");
    }
}


void DeribitClient::send_auth(const std::string& api_key, const std::string& api_secret) {
    json msg = {
        {"jsonrpc", "2.0"},
        {"id", generate_id("auth")},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", api_key},
            {"client_secret", api_secret}
        }}
    };

    send_message(msg);
    // std::cout << "Sent authentication request\n";
    is_authenticated_ = true;
    spdlog::info(" Sent authentication request");
}



void DeribitClient::place_order(
     const std::string& instrument,
    std::optional<double> amount,
    std::optional<double> contracts,
    std::string type,
    std::optional<double> price,
    std::optional<std::string> time_in_force,
    std::optional<bool> post_only,
    std::optional<bool> reduce_only,
    std::optional<std::string> label,
    std::optional<double> trigger_price,
    std::optional<std::string> trigger
) {
    json params;
    params["instrument_name"] = instrument;
    if (amount) params["amount"] = *amount;
    if (contracts) params["contracts"] = *contracts;
    params["type"] = type;
    if (price) params["price"] = *price;
    if (time_in_force) params["time_in_force"] = *time_in_force;
    if (post_only) params["post_only"] = *post_only;
    if (reduce_only) params["reduce_only"] = *reduce_only;
    if (label) params["label"] = *label;
    if (trigger_price) params["trigger_price"] = *trigger_price;
    if (trigger) params["trigger"] = *trigger;

    json msg = {
        {"jsonrpc", "2.0"},
        {"id",generate_id("order")},
        {"method", "private/buy"},  // or sell based on future param
        {"params", params}
    };

    send_message(msg);

    spdlog::info("Sent  order request for {}", instrument);

}

void DeribitClient::modify_order(
    const std::string& order_id,
    std::optional<double> amount,
    std::optional<double> price,
    std::optional<std::string> post_only,
    std::optional<std::string> time_in_force,
    std::optional<std::string> label
) {
    json params = {
        {"order_id", order_id}
    };

    if (amount.has_value()) params["amount"] = amount.value();
    if (price.has_value()) params["price"] = price.value();
    if (post_only.has_value()) params["post_only"] = (post_only.value() == "true");
    if (time_in_force.has_value()) params["time_in_force"] = time_in_force.value();
    if (label.has_value()) params["label"] = label.value();

    json msg = {
        {"jsonrpc", "2.0"},
        {"id", generate_id("modify")},
        {"method", "private/edit"},
        {"params", params}
    };

    send_message(msg);
    // std::cout << "✏️ Sent modify request for Order ID: " << order_id << "\n";
    spdlog::info("✏️ Sent modify request for Order ID: {}", order_id);
}

void DeribitClient::cancel_order(const std::string& order_id) {
    json msg = {
        {"jsonrpc", "2.0"},
        {"id", generate_id("cancel")},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id}
        }}
    };

    send_message(msg);  // using the generic write function
    std::cout << " Sent cancel request for Order ID: " << order_id << "\n";
}

void DeribitClient::cancel_all(
    std::optional<std::string> type,
    std::optional<std::string> instrument_name,
    std::optional<std::string> kind
) {
    json msg = {
        {"jsonrpc", "2.0"},
        {"id",generate_id("cancel_all")},
        {"method", "private/cancel_all"},
        {"params", json::object()}
    };

    if (type) msg["params"]["type"] = *type;
    if (instrument_name) msg["params"]["instrument_name"] = *instrument_name;
    if (kind) msg["params"]["kind"] = *kind;

    send_message(msg);
    std::cout << "Sent cancel all request\n";
}


void DeribitClient::get_orderbook(const std::string& instrument_name, int depth) {
    json params = {
        {"instrument_name", instrument_name},
        {"depth", depth}
    };

    json msg = {
        {"jsonrpc", "2.0"},
        {"id", generate_id("get_orderbook")},
        {"method", "public/get_order_book"},
        {"params", params}
    };

    send_message(msg);
    std::cout << " Requested order book for: " << instrument_name << " (Depth: " << depth << ")\n";
}

void DeribitClient::get_positions(const std::string& currency, const std::string& kind) {
    json params = {
        {"currency", currency},
        {"kind", kind}  // "future", "option", or "any"
    };

    json msg = {
        {"jsonrpc", "2.0"},
        {"id", generate_id("get_positions")},
        {"method", "private/get_positions"},
        {"params", params}
    };

    send_message(msg);
    // std::cout << "📦 Requested positions for currency: " << currency << ", kind: " << kind << "\n";
    spdlog::info("📦 Requested positions for currency: {}, kind: {}", currency, kind);
}

void DeribitClient::subscribe_book(const std::string& instrument) {
    if (subscribed_symbols_.count(instrument)) {
        std::cout << " Already subscribed to " << instrument << "\n";
        return;
    }
    
     std::string scope = is_authenticated_ ? "private" : "public";
    json msg = {
        {"jsonrpc", "2.0"},
        {"id", generate_id("subscribe")},
        {"method", scope +"/subscribe"},
        {"params", {
            {"channels", { "book." + instrument  }}
        }}
    };

    send_message(msg);
    subscribed_symbols_.insert(instrument);
    std::cout << " Subscribed to " << instrument << " order book\n";
    spdlog::info(" Subscribed to {} order book", instrument);
}

void DeribitClient::unsubscribe(const std::string& symbol) {
    if (!subscribed_symbols_.count(symbol)) {
        std::cout << " Not subscribed to " << symbol << "\n";
        return;
    }
     std::string scope = is_authenticated_ ? "private" : "public";

    json msg = {
        {"jsonrpc", "2.0"},
        {"id", generate_id("unsubscribe")},
        {"method", scope + "/unsubscribe"},
        {"params", {
            {"channels", { "book." + symbol  }}
        }}
    };

    send_message(msg);
    subscribed_symbols_.erase(symbol);
    std::cout << " Unsubscribed from " << symbol << "\n";
    spdlog::info(" Unsubscribed from {}", symbol);
}

void DeribitClient::list_subscriptions() const {
    std::cout << " Subscribed Symbols:\n";
    if (subscribed_symbols_.empty()) {
        std::cout << "  None\n";
    } else {
        for (const auto& symbol : subscribed_symbols_) {
            std::cout << "  - " << symbol << "\n";
        }
    }
}


