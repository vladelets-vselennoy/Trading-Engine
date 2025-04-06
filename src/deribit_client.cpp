// deribit_client.cpp
//[TODO: add message parsing  try using name or ny entity returned in json object similar name , queue for write operations , error logging ]
#include "deribit_client.hpp"
#include <boost/asio/connect.hpp>
#include <iostream>
std::queue<std::string> DeribitClient::receive_queue;
std::queue<std::string> DeribitClient::send_queue;
std::mutex DeribitClient::receive_queue_mutex;
std::mutex DeribitClient::send_queue_mutex;
DeribitClient::DeribitClient(const std::string& host, const std::string& port, const std::string& target)
    : host_(host),
      port_(port),
      target_(target),
      ctx_(ssl::context::tlsv12_client),
      ws_(ioc_, ctx_),
       timer_(ioc_) {
    ctx_.set_default_verify_paths();
}

void DeribitClient::connect() {
    tcp::resolver resolver(ioc_);
    auto const results = resolver.resolve(host_, port_);
    boost::asio::connect(beast::get_lowest_layer(ws_), results.begin(), results.end());
    ws_.next_layer().handshake(ssl::stream_base::client);
    ws_.handshake(host_, target_);
    std::cout << "✅ Connected to Deribit WebSocket\n";
}

void DeribitClient::start_receiving() {
    recv_thread_ = std::thread(&DeribitClient::receive_loop, this);
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
                std::cout << "📩 Received: " << message << "\n";
            }
            buffer.clear();
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Receive Error: " << e.what() << "\n";
    }
}

void DeribitClient::process_messages() {
    while (true) {
        if (!receive_queue.empty()) {
            std::lock_guard<std::mutex> lock(receive_queue_mutex);
            std::string message = receive_queue.front();
            receive_queue.pop();
            // Process the message (you can call MessageParser::parse_and_print here)
            std::cout << "Processing received message: " << message << "\n";
        }
    }
}

// void DeribitClient::receive_loop() {
//     try {
//         beast::flat_buffer buffer;
//         while (true) {
//             ws_.read(buffer);
//             std::lock_guard<std::mutex> lock(mtx_);
//             // std::cout << "📩 " << beast::buffers_to_string(buffer.data()) << "\n";
//              auto msg_str = beast::buffers_to_string(buffer.data());
//              MessageParser::parse_and_print(msg_str);

//             buffer.clear();
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "❌ Receive Error: " << e.what() << "\n";
//     }
// }
void DeribitClient::send_message(const json& msg, std::chrono::milliseconds timeout) {
    // Set up the timer with the specified timeout or default 1000ms
    timer_.expires_after(timeout);  // Set timeout duration
    std::cout << "🕒 Timeout set to: " << timeout.count() << " ms\n";

    // Asynchronously wait for the timeout while sending the message
    boost::asio::post(ioc_, [this, msg]() {
        try {
            std::lock_guard<std::mutex> lock(mtx_);
            ws_.write(boost::asio::buffer(msg.dump()));
            std::cout << "➡️ Sent: " << msg.dump() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "❌ Send Error: " << e.what() << "\n";
        }
    });

    // Wait for the timer expiration
    boost::system::error_code ec;
    timer_.async_wait([this](const boost::system::error_code& error) {
        if (error != boost::asio::error::operation_aborted) {
            std::cerr << "❌ Write Timeout Occurred\n";
            // Handle timeout behavior here (e.g., notify user, log error, etc.)
        }
    });

    // Run the io_context to manage both sending the message and checking for timeout
    ioc_.run_for(timeout);
}
// void DeribitClient::send_message(const json& msg) {
//      std::string message = msg.dump();
//     {
//         // Lock the send queue to safely push the message
//         std::lock_guard<std::mutex> lock(send_queue_mutex);
//         send_queue.push(message);
//         std::cout << "➡️ Added to send queue: " << message << "\n";
//     }
//     // std::lock_guard<std::mutex> lock(mtx_);
//     // ws_.write(boost::asio::buffer(msg.dump()));
//     // std::cout << "➡️ Sent: " << msg.dump() << "\n";
// }

void DeribitClient::send_messages() {
    while (true) {
        if (!send_queue.empty()) {
            std::lock_guard<std::mutex> lock(send_queue_mutex);
            std::string message = send_queue.front();
            send_queue.pop();
            // Send the message via WebSocket
            ws_.write(boost::asio::buffer(message));
            std::cout << "➡️ Sent: " << message << "\n";
        }
    }
}

void DeribitClient::close() {
    // std::lock_guard<std::mutex> lock(mtx_);
    std::lock_guard<std::mutex> lock(send_queue_mutex);
    ws_.close(websocket::close_code::normal);
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
    std::cout << "👋 Disconnected.\n";
}


void DeribitClient::send_auth(const std::string& api_key, const std::string& api_secret) {
    json msg = {
        {"jsonrpc", "2.0"},
        {"id", ++msg_id_},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", api_key},
            {"client_secret", api_secret}
        }}
    };

    send_message(msg);
    std::cout << "🔐 Sent authentication request\n";
}



// void DeribitClient::place_order(const std::string& instrument, double amount, double price) {
//     json msg = {
//         {"jsonrpc", "2.0"},
//         {"id", ++msg_id_},
//         {"method", "private/buy"},
//         {"params", {
//             {"instrument_name", instrument},
//             {"amount", amount},
//             {"type", "limit"},
//             {"price", price},
//             {"post_only", true}
//         }}
//     };

//     std::lock_guard<std::mutex> lock(mtx_);
//     ws_.write(boost::asio::buffer(msg.dump()));
//     std::cout << "📝 Placed limit buy order: " << amount << " " << instrument << " @ $" << price << "\n";
// }

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
        {"id", ++msg_id_},
        {"method", "private/buy"},  // or sell based on future param
        {"params", params}
    };

    send_message(msg);

    std::cout << "📝 Sent flexible order request for " << instrument << "\n";
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
        {"id", ++msg_id_},
        {"method", "private/edit"},
        {"params", params}
    };

    send_message(msg);
    std::cout << "✏️ Sent modify request for Order ID: " << order_id << "\n";
}

void DeribitClient::cancel_order(const std::string& order_id) {
    json msg = {
        {"jsonrpc", "2.0"},
        {"id", ++msg_id_},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id}
        }}
    };

    send_message(msg);  // using the generic write function
    std::cout << "🗑️ Sent cancel request for Order ID: " << order_id << "\n";
}

void DeribitClient::cancel_all(
    std::optional<std::string> type,
    std::optional<std::string> instrument_name,
    std::optional<std::string> kind
) {
    json msg = {
        {"jsonrpc", "2.0"},
        {"id", ++msg_id_},
        {"method", "private/cancel_all"},
        {"params", json::object()}
    };

    if (type) msg["params"]["type"] = *type;
    if (instrument_name) msg["params"]["instrument_name"] = *instrument_name;
    if (kind) msg["params"]["kind"] = *kind;

    send_message(msg);
    std::cout << "🗑️ Sent cancel all request\n";
}


void DeribitClient::get_orderbook(const std::string& instrument_name, int depth) {
    json params = {
        {"instrument_name", instrument_name},
        {"depth", depth}
    };

    json msg = {
        {"jsonrpc", "2.0"},
        {"id", ++msg_id_},
        {"method", "public/get_order_book"},
        {"params", params}
    };

    send_message(msg);
    std::cout << "📊 Requested order book for: " << instrument_name << " (Depth: " << depth << ")\n";
}

void DeribitClient::get_positions(const std::string& currency, const std::string& kind) {
    json params = {
        {"currency", currency},
        {"kind", kind}  // "future", "option", or "any"
    };

    json msg = {
        {"jsonrpc", "2.0"},
        {"id", ++msg_id_},
        {"method", "private/get_positions"},
        {"params", params}
    };

    send_message(msg);
    std::cout << "📦 Requested positions for currency: " << currency << ", kind: " << kind << "\n";
}

void DeribitClient::subscribe_book(const std::string& instrument) {
    if (subscribed_symbols_.count(instrument)) {
        std::cout << "🔁 Already subscribed to " << instrument << "\n";
        return;
    }
    json msg = {
        {"jsonrpc", "2.0"},
        {"id", ++msg_id_},
        {"method", "public/subscribe"},
        {"params", {
            {"channels", { "book." + instrument  }}
        }}
    };

    send_message(msg);
    std::cout << "📡 Subscribed to " << instrument << " order book\n";
}

void DeribitClient::unsubscribe(const std::string& symbol) {
    if (!subscribed_symbols_.count(symbol)) {
        std::cout << "❌ Not subscribed to " << symbol << "\n";
        return;
    }

    json msg = {
        {"jsonrpc", "2.0"},
        {"id", ++msg_id_},
        {"method", "public/unsubscribe"},
        {"params", {
            {"channels", { "book." + symbol  }}
        }}
    };

    send_message(msg);
    subscribed_symbols_.erase(symbol);
    std::cout << "🚫 Unsubscribed from " << symbol << "\n";
}

void DeribitClient::list_subscriptions() const {
    std::cout << "📋 Subscribed Symbols:\n";
    if (subscribed_symbols_.empty()) {
        std::cout << "  None\n";
    } else {
        for (const auto& symbol : subscribed_symbols_) {
            std::cout << "  - " << symbol << "\n";
        }
    }
}


