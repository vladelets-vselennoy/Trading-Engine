// message_parser.hpp
#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include<fmt/format.h>
#include <fmt/core.h>


class MessageParser {
public:
MessageParser();
     void parse_and_print(const std::string& raw_msg);
private:
void handle_auth(const nlohmann::json& msg);
    void handle_buy(const nlohmann::json& msg);
    void handle_cancel(const nlohmann::json& msg);
    void handle_subscription(const nlohmann::json& msg);
    void handle_cancel_all(const nlohmann::json& msg);
    void handle_positions(const nlohmann::json& msg);
    void handle_orderbook(const nlohmann::json& msg);
    void handle_modify(const nlohmann::json& msg);
    void handle_unsubscribe(const nlohmann::json& msg);
    void handle_subscription_response(const nlohmann::json& msg);


     std::unordered_map<std::string, std::function<void(const nlohmann::json&)>> handlers_;

};
