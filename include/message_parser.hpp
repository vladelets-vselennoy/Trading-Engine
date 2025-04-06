// message_parser.hpp
#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/core.h>

/// @brief Parser for handling Deribit WebSocket API messages
/// Provides methods to parse and handle different types of responses from the exchange
class MessageParser {
public:
    /// @brief Constructs a MessageParser and initializes message handlers
    MessageParser();

    /// @brief Parses a raw message and routes it to appropriate handler
    /// @param raw_msg The raw JSON message string from WebSocket
    void parse_and_print(const std::string& raw_msg);

private:
    /// @brief Handles authentication response messages
    /// @param msg The parsed JSON authentication response
    void handle_auth(const nlohmann::json& msg);

    /// @brief Handles buy order response messages
    /// @param msg The parsed JSON buy order response
    void handle_buy(const nlohmann::json& msg);

    /// @brief Handles order cancellation response messages
    /// @param msg The parsed JSON cancel order response
    void handle_cancel(const nlohmann::json& msg);

    /// @brief Handles subscription response messages
    /// @param msg The parsed JSON subscription response
    void handle_subscription(const nlohmann::json& msg);

    /// @brief Handles cancel all orders response messages
    /// @param msg The parsed JSON cancel all response
    void handle_cancel_all(const nlohmann::json& msg);

    /// @brief Handles position information response messages
    /// @param msg The parsed JSON positions response
    void handle_positions(const nlohmann::json& msg);

    /// @brief Handles orderbook data response messages
    /// @param msg The parsed JSON orderbook response
    void handle_orderbook(const nlohmann::json& msg);

    /// @brief Handles order modification response messages
    /// @param msg The parsed JSON modify order response
    void handle_modify(const nlohmann::json& msg);

    /// @brief Handles unsubscribe response messages
    /// @param msg The parsed JSON unsubscribe response
    void handle_unsubscribe(const nlohmann::json& msg);

    /// @brief Handles real-time subscription update messages
    /// @param msg The parsed JSON subscription update
    void handle_subscription_response(const nlohmann::json& msg);

    /// @brief Map of message handlers indexed by message type
    std::unordered_map<std::string, std::function<void(const nlohmann::json&)>> handlers_;
};
