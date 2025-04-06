// message_parser.cpp
#include "message_parser.hpp"
#include <iostream>
#include <iomanip>

using json = nlohmann::json;

void MessageParser::parse_and_print(const std::string& raw_msg) {
    try {
        auto msg = json::parse(raw_msg);

        // Basic Response Info
        if (msg.contains("error")) {
            std::cerr << "❌ Error: " << msg["error"].dump(2) << "\n";
            return;
        }

        if (msg.contains("result")) {
            std::cout << "✅ Result:\n" << std::setw(2) << msg["result"] << "\n";
        }

        if (msg.contains("method")) {
            std::string method = msg["method"];
            std::cout << "📩 Event: " << method << "\n";

            if (msg.contains("params")) {
                std::cout << "🔍 Params:\n" << std::setw(2) << msg["params"] << "\n";
            }
        }

        if (msg.contains("id")) {
            std::cout << "🆔 Message ID: " << msg["id"] << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "🚨 Failed to parse message: " << e.what() << "\n";
    }
}
