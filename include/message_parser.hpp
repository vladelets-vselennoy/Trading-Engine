// message_parser.hpp
#pragma once

#include <string>
#include <nlohmann/json.hpp>

class MessageParser {
public:
    static void parse_and_print(const std::string& raw_msg);
};
