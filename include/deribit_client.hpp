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
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <memory>
#include <set>


namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

/// @brief Client class for interacting with the Deribit cryptocurrency exchange API
class DeribitClient {
public:
    /// @brief Constructs a new DeribitClient
    /// @param host The hostname of the Deribit server
    /// @param port The port number for the connection
    /// @param target The WebSocket endpoint path
    /// @param server_port The local server port number
    DeribitClient(const std::string& host, const std::string& port, const std::string& target, uint16_t server_port);  // Changed from int to uint16_t for consistency

    /// @brief Establishes a WebSocket connection to the Deribit server
    void connect();

    /// @brief Starts the message receiving thread
    void start_receiving();

    /// @brief Starts the message sending thread
    void start_sending();

    /// @brief Authenticates with the Deribit API
    /// @param api_key The API key from Deribit
    /// @param api_secret The API secret from Deribit
    void send_auth(const std::string& api_key, const std::string& api_secret);

    /// @brief Subscribes to orderbook updates for an instrument
    /// @param instrument The instrument identifier (e.g., "BTC-PERPETUAL")
    void subscribe_book(const std::string& instrument);

    /// @brief Cancels an order by ID
    /// @param order_id The order ID to cancel
    void cancel_order(const std::string& order_id);

    /// @brief Closes the WebSocket connection
    void close();

    /// @brief Cancels all orders for a specific instrument type
    /// @param instrument_type The type of instrument
    void cancel_all(const std::string& instrument_type);

    /// @brief Gets the orderbook for an instrument
    /// @param instrument_name The instrument name
    /// @param depth The depth of the orderbook (default: 10)
    void get_orderbook(const std::string& instrument_name, int depth = 10);

    /// @brief Gets positions for a currency
    /// @param currency The currency to get positions for
    /// @param kind The kind of positions (default: "any")
    void get_positions(const std::string& currency, const std::string& kind = "any");

    /// @brief Unsubscribes from a symbol's updates
    /// @param symbol The symbol to unsubscribe from
    void unsubscribe(const std::string& symbol);

    /// @brief Generates a unique message ID
    /// @param function_name The name of the calling function
    /// @return A unique message identifier
    std::string generate_id(const std::string& function_name);
     bool is_authenticated_ = false;

    /// @brief Lists all active subscriptions
    void list_subscriptions() const;

    /// @brief Initializes the logging system
    static void init_logger();

     void process_messages();
     void send_messages();
     void start_message_processing();
     void handle_error(const std::string& context, const std::exception& e);

    /// @brief Places an order on the exchange
    /// @param instrument The instrument to trade
    /// @param amount Optional amount in currency
    /// @param contracts Optional number of contracts
    /// @param type Order type (e.g., "limit", "market")
    /// @param price Optional limit price
    /// @param time_in_force Optional TIF instruction
    /// @param post_only Optional post-only flag
    /// @param reduce_only Optional reduce-only flag
    /// @param label Optional order label
    /// @param trigger_price Optional trigger price
    /// @param trigger Optional trigger type
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

    /// @brief Modifies an existing order
    /// @param order_id The order ID to modify
    /// @param amount Optional new amount
    /// @param price Optional new price
    /// @param post_only Optional post-only setting
    /// @param time_in_force Optional new TIF
    /// @param label Optional new label
    void modify_order(
    const std::string& order_id,
    std::optional<double> amount = std::nullopt,
    std::optional<double> price = std::nullopt,
    std::optional<std::string> post_only = std::nullopt,
    std::optional<std::string> time_in_force = std::nullopt,
    std::optional<std::string> label = std::nullopt
    );

    /// @brief Cancels all orders matching criteria
    /// @param type Optional order type filter
    /// @param instrument_name Optional instrument filter
    /// @param kind Optional kind filter
    void cancel_all(
    std::optional<std::string> type = std::nullopt,
    std::optional<std::string> instrument_name = std::nullopt,
    std::optional<std::string> kind = std::nullopt
    );

    /// @brief Starts the local WebSocket server
    void start_server();

    /// @brief Stops the local WebSocket server
    void stop_server();

private:
    /// @brief Main receive loop for WebSocket messages
    void receive_loop();

    /// @brief Sends a JSON message through WebSocket
    /// @param msg The message to send
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

    // Server-related members
    uint16_t server_port_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::set<std::shared_ptr<websocket::stream<tcp::socket>>> sessions_;
    std::mutex sessions_mutex_;

    /// @brief Handles new client connections
    void do_accept();

    /// @brief Broadcasts message to all connected clients
    void broadcast_message(const std::string& message);

    /// @brief Removes a client session
    void remove_session(std::shared_ptr<websocket::stream<tcp::socket>> session);

    // Add new subscription tracking structures
    struct ClientSession {
        std::shared_ptr<websocket::stream<tcp::socket>> socket;
        std::set<std::string> subscriptions;  // Channels this client is subscribed to
        std::chrono::steady_clock::time_point last_active;
    };

    // Map of session ID to client session info
    std::map<std::string, std::shared_ptr<ClientSession>> client_sessions_;
    std::mutex client_sessions_mutex_;

    // Map of channel to set of subscribed session IDs
    std::map<std::string, std::set<std::string>> channel_subscribers_;
    std::mutex channel_subscribers_mutex_;

    /// @brief Adds a channel subscription for a client
    /// @param session_id The client's session ID
    /// @param channel The channel to subscribe to
    void add_client_subscription(const std::string& session_id, const std::string& channel);

    /// @brief Removes a channel subscription for a client
    /// @param session_id The client's session ID
    /// @param channel The channel to unsubscribe from
    void remove_client_subscription(const std::string& session_id, const std::string& channel);

    /// @brief Broadcasts message to clients subscribed to a specific channel
    /// @param channel The channel to broadcast to
    /// @param message The message to broadcast
    void broadcast_to_channel(const std::string& channel, const std::string& message);

    /// @brief Cleans up inactive client sessions
    void cleanup_inactive_sessions();

    /// @brief Handles a new WebSocket session
    /// @param session The WebSocket session to handle
    void handle_session(std::shared_ptr<websocket::stream<tcp::socket>> session);

    /// @brief Updates the last active timestamp for a client session
    /// @param session_id The ID of the session to update
    void update_client_activity(const std::string& session_id);

    /// @brief Removes a client session and all its subscriptions
    /// @param session_id The ID of the session to remove
    void remove_client_session(const std::string& session_id);
    
};
