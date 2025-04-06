// deribit_client.hpp
#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/io_context.hpp>
#include <nlohmann/json.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <optional>
#include "message_parser.hpp"
#include <unordered_set>
#include <queue>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

class DeribitClient {
public:
    DeribitClient(const std::string& host, const std::string& port, const std::string& target);

    void connect();
    void start_receiving();
    void start_sending();
    void send_auth(const std::string& api_key, const std::string& api_secret);
    void subscribe_book(const std::string& instrument);
    void cancel_order(const std::string& order_id);
    // void place_order(const std::string& instrument, double amount, double price);
    void close();
    void cancel_all(const std::string& instrument_type);
    void get_orderbook(const std::string& instrument_name, int depth = 10);
    void get_positions(const std::string& currency, const std::string& kind = "any");
    void unsubscribe(const std::string& symbol);
    std::string generate_id(const std::string& function_name);
     bool is_authenticated_ = false;

    void list_subscriptions() const;
     static void init_logger();

     void process_messages();
     void send_messages();
     void start_message_processing();
     void handle_error(const std::string& context, const std::exception& e);



    void place_order(
        const std::string& instrument,
        std::optional<double> amount = std::nullopt,
        std::optional<double> contracts = std::nullopt,
        std::string type = "limit",
        std::optional<double> price = std::nullopt,
        std::optional<std::string> time_in_force = std::nullopt,
        std::optional<bool> post_only = std::nullopt,
        std::optional<bool> reduce_only = std::nullopt,
        std::optional<std::string> label = std::nullopt,
        std::optional<double> trigger_price = std::nullopt,
        std::optional<std::string> trigger = std::nullopt
    );
    void modify_order(
    const std::string& order_id,
    std::optional<double> amount = std::nullopt,
    std::optional<double> price = std::nullopt,
    std::optional<std::string> post_only = std::nullopt,
    std::optional<std::string> time_in_force = std::nullopt,
    std::optional<std::string> label = std::nullopt
    );
    void cancel_all(
    std::optional<std::string> type = std::nullopt,
    std::optional<std::string> instrument_name = std::nullopt,
    std::optional<std::string> kind = std::nullopt
    );
    
    




private:
    void receive_loop();
     void send_message(const json& msg);  
    std::string host_, port_, target_;
    boost::asio::io_context ioc_;
    ssl::context ctx_;
    websocket::stream<beast::ssl_stream<tcp::socket>> ws_;
    std::mutex mtx_;
    int msg_id_ = 1;
    std::thread recv_thread_;
    std::thread send_thread_;
    std::thread process_thread_;
    std::unordered_set<std::string> subscribed_symbols_;
       // Queues for storing messages
    static std::queue<std::string> receive_queue;
    static std::queue<std::string> send_queue;

    // Mutexes for protecting access to the queues
    static std::mutex receive_queue_mutex;
    static std::mutex send_queue_mutex;
   
    static std::shared_ptr<spdlog::logger> logger_;


     boost::asio::steady_timer timer_;  
     MessageParser parser_;
    
};
