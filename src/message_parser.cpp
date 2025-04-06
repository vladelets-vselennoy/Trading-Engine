// message_parser.cpp
#include "message_parser.hpp"
#include <iostream>
#include <iomanip>


using json = nlohmann::json;

MessageParser::MessageParser() {
    handlers_["auth"] = [this](const json& msg) { handle_auth(msg); };
    handlers_["order"] = [this](const json& msg) { handle_buy(msg); };
    handlers_["cancel"] = [this](const json& msg) { handle_cancel(msg); };
    handlers_["subscribe"] = [this](const json& msg) { handle_subscription(msg); };
    handlers_["cancel_all"]= [this](const json& msg) { handle_cancel_all(msg); };
    handlers_["get_positions"] = [this](const json& msg) { handle_positions(msg); };
    handlers_["get_orderbook"] = [this](const json& msg) { handle_orderbook(msg); };
    handlers_["modify"] = [this](const json& msg) { handle_modify(msg); };
    handlers_["unsubscribe"] = [this](const json& msg) { handle_unsubscribe(msg); };

}

void MessageParser::parse_and_print(const std::string& raw_msg) {
    try {
        auto msg = json::parse(raw_msg);

        if (!msg.contains("id")) {
               if (msg.contains("method")) {
            std::string method = msg["method"];
            
            if (method == "subscription") {
                // Call subscription-specific handling function
                handle_subscription_response(msg);
                return;
            }
        }
            std::cerr << "Missing 'id' field in message:\n" << std::setw(2) << msg << "\n";
            return;
        }

        std::string id = msg["id"];
        std::string function_name;

        // Extract function name from id (e.g., auth_1 → auth)
        size_t pos = id.find('#');
        if (pos != std::string::npos) {
            function_name = id.substr(0, pos);
        } else {
            std::cerr << "Invalid id format (expected function_index): " << id << "\n";
            std::cout << std::setw(2) << msg << "\n";
            return;
        }

        if (msg.contains("error") && !msg["error"].is_null()) {
            std::cerr << "[Error - " << function_name << "]\n";
            std::cerr << "Code    : " << msg["error"]["code"] << "\n";
            std::cerr << "Message : " << msg["error"]["message"] << "\n";
            return;
        }

        auto it = handlers_.find(function_name);
        if (it != handlers_.end()) {
            it->second(msg); // Dispatch to handler
        } else {
            std::cout << "[No handler for id prefix: " << function_name << "]\n";
            std::cout << std::setw(2) << msg << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Failed to parse message: " << e.what() << "\n";
    }
}

void MessageParser::handle_auth(const json&msg){
    std::cout << " Auth Response:\n";
    if (msg.contains("error")) {
        std::cerr << "Auth Error: " << msg["error"].dump(2) << "\n";
        return;
    }
    if (msg.contains("result")) {
        std::cout << "Successful Authentication" << "\n";
    } else {
        std::cerr << "Unexpected Auth Response Format\n";
    }
}

void MessageParser::handle_buy(const json& msg) {
    fmt::print("Buy Order Response:\n");

    if (msg.contains("error")) {
        fmt::print("Buy Error: {}\n", msg["error"].dump(2));
        return;
    }

    if (!msg.contains("result")) {
        fmt::print("Unexpected Buy Response Format: Missing 'result'\n");
        return;
    }

    const auto& order = msg["result"]["order"];

    fmt::print("Order ID: {}\n", order.value("order_id", "N/A"));
    fmt::print("Instrument: {}\n", order.value("instrument_name", "N/A"));
    fmt::print("Direction: {}\n", order.value("direction", "N/A"));
    fmt::print("Amount: {}\n", order.value("amount", 0.0));
    fmt::print("Price: {}\n", order.value("price", 0.0));
    fmt::print("Order Type: {}\n", order.value("order_type", "N/A"));
    fmt::print("Order State: {}\n", order.value("order_state", "N/A"));
    fmt::print("Filled Amount: {}\n", order.value("filled_amount", 0.0));
    fmt::print("Average Price: {}\n", order.value("average_price", 0.0));
    fmt::print("Creation Timestamp: {}\n", order.value("creation_timestamp", 0));
    fmt::print("Last Update Timestamp: {}\n", order.value("last_update_timestamp", 0));

    fmt::print("Buy Order Handled Successfully.\n");
}

void MessageParser::handle_cancel(const nlohmann::json& msg) {
    fmt::print("Cancel Order Response:\n");

    if (msg.contains("error")) {
        fmt::print("Error during canceling order:\n{}\n", msg["error"].dump(2));
        return;
    }

    if (!msg.contains("result")) {
        fmt::print("Invalid cancel order response: missing 'result'\n");
        return;
    }

    const auto& result = msg["result"];

    fmt::print("Order ID: {}\n", result.value("order_id", "N/A"));
    fmt::print("Instrument: {}\n", result.value("instrument_name", "N/A"));
    fmt::print("Direction: {}\n", result.value("direction", "N/A"));
    fmt::print("Amount: {}\n", result.value("amount", 0.0));
    fmt::print("Trigger: {}\n", result.value("trigger", "N/A"));
    fmt::print("Trigger Price: {}\n", result.value("trigger_price", 0.0));
    fmt::print("Order Type: {}\n", result.value("order_type", "N/A"));
    fmt::print("Order State: {}\n", result.value("order_state", "N/A"));
    fmt::print("Reduce Only: {}\n", result.value("reduce_only", false));
    fmt::print("Post Only: {}\n", result.value("post_only", false));
    fmt::print("API Order: {}\n", result.value("api", false));
    fmt::print("Creation Time: {}\n", result.value("creation_timestamp", 0));
    fmt::print("Last Update: {}\n", result.value("last_update_timestamp", 0));
}

void MessageParser::handle_cancel_all(const json& msg) {
    if (msg.contains("error")) {
        fmt::print("Cancel All Error: {}\n", msg["error"].dump(2));
        return;
    }

    if (msg.contains("result")) {
        int cancelled_count = msg["result"];
        fmt::print("Cancel All Successful: {} orders cancelled.\n", cancelled_count);
    } else {
        fmt::print("Unexpected Cancel All Response Format\n");
    }
}

void MessageParser::handle_orderbook(const json& msg) {
    if (msg.contains("error")) {
        fmt::print("Orderbook Error: {}\n", msg["error"].dump(2));
        return;
    }

    if (!msg.contains("result")) {
        fmt::print("Unexpected Orderbook Response Format\n");
        return;
    }

    const auto& result = msg["result"];
    fmt::print("Orderbook for {}\n", result.value("instrument_name", "N/A"));
    fmt::print("State: {}\n", result.value("state", "N/A"));
    fmt::print("Last Price: {}\n", result.value("last_price", 0.0));
    fmt::print("Index Price: {}\n", result.value("index_price", 0.0));
    fmt::print("Mark Price: {}\n", result.value("mark_price", 0.0));
    fmt::print("Funding 8h: {}\n", result.value("funding_8h", 0.0));
    fmt::print("Current Funding: {}\n", result.value("current_funding", 0.0));
    fmt::print("Open Interest: {}\n", result.value("open_interest", 0.0));

    // Stats block
    if (result.contains("stats")) {
        const auto& stats = result["stats"];
        fmt::print("Stats - Volume: {}, Price Change: {}, High: {}, Low: {}\n",
            stats.value("volume", 0.0),
            stats.value("price_change", 0.0),
            stats.value("high", 0.0),
            stats.value("low", 0.0));
    }

    // Bids
    if (result.contains("bids") && result["bids"].is_array()) {
        fmt::print("Top Bids:\n");
        for (const auto& bid : result["bids"]) {
           if (bid.is_array() && bid.size() == 2) {
            // Make sure both bid[0] and bid[1] are numbers
            if (bid[0].is_number() && bid[1].is_number()) {
                fmt::print("  Price: {}, Amount: {}\n", bid[0], bid[1]);
            } else {
                fmt::print("  Invalid bid data (non-number values found).\n");
            }
        }
    }
    }

    // Asks
    if (result.contains("asks") && result["asks"].is_array() && !result["asks"].empty()) {
        fmt::print("Top Asks:\n");
        for (const auto& ask : result["asks"]) {
            if (ask.is_array() && ask.size() == 2) {
                fmt::print("  Price: {}, Amount: {}\n", ask[0], ask[1]);
            }
        }
    } else {
        fmt::print("No asks available.\n");
    }
}

void MessageParser::handle_positions(const json& msg) {
    if (msg.contains("error")) {
        fmt::print("Positions Error: {}\n", msg["error"].dump(2));
        return;
    }

    if (!msg.contains("result") || !msg["result"].is_array()) {
        fmt::print("Unexpected Positions Response Format\n");
        return;
    }

    const auto& positions = msg["result"];
    for (const auto& position : positions) {
        fmt::print("Instrument: {}\n", position.value("instrument_name", "N/A"));
        fmt::print("Kind: {}\n", position.value("kind", "N/A"));
        fmt::print("Direction: {}\n", position.value("direction", "N/A"));
        fmt::print("Size: {}\n", position.value("size", 0));
        fmt::print("Leverage: {}\n", position.value("leverage", 0));
        fmt::print("Mark Price: {}\n", position.value("mark_price", 0.0));
        fmt::print("Index Price: {}\n", position.value("index_price", 0.0));
        fmt::print("Average Price: {}\n", position.value("average_price", 0.0));
        fmt::print("Size Currency: {}\n", position.value("size_currency", 0.0));
        fmt::print("Delta: {}\n", position.value("delta", 0.0));
        fmt::print("Total PnL: {}\n", position.value("total_profit_loss", 0.0));
        fmt::print("Floating PnL: {}\n", position.value("floating_profit_loss", 0.0));
        fmt::print("Realized PnL: {}\n", position.value("realized_profit_loss", 0.0));
        fmt::print("Realized Funding: {}\n", position.value("realized_funding", 0.0));
        // fmt::print("Est. Liquidation Price: {}\n", position.value("estimated_liquidation_price", 0.0));
        fmt::print("Initial Margin: {}\n", position.value("initial_margin", 0.0));
        fmt::print("Maintenance Margin: {}\n", position.value("maintenance_margin", 0.0));
        fmt::print("Open Orders Margin: {}\n", position.value("open_orders_margin", 0.0));
        fmt::print("Interest Value: {}\n", position.value("interest_value", 0.0));
        fmt::print("Settlement Price: {}\n", position.value("settlement_price", 0.0));
        fmt::print("------------------------------------------------------------\n");
    }
    fmt::print("Response of positions request printed");
}

void MessageParser::handle_subscription(const json& msg) {
    fmt::print("Subscription Response:\n");

    if (msg.contains("error")) {
        fmt::print(" Subscription Error: {}\n", msg["error"].dump(2));
        return;
    }

    if (!msg.contains("result") || !msg["result"].is_array()) {
        fmt::print("Invalid subscription response format.\n");
        return;
    }

    const auto& channels = msg["result"];
    for (const auto& channel : channels) {
        fmt::print("Subscribed to channel: {}\n", channel.get<std::string>());
    }
}

void MessageParser::handle_unsubscribe(const json& msg) {
    fmt::print("Unsubscription Response:\n");

    if (msg.contains("error")) {
        fmt::print("Unsubscribe Error: {}\n", msg["error"].dump(2));
        return;
    }

    if (!msg.contains("result") || !msg["result"].is_array()) {
        fmt::print("Invalid unsubscription response format.\n");
        return;
    }

    const auto& channels = msg["result"];
    for (const auto& channel : channels) {
        fmt::print("Unsubscribed from channel: {}\n", channel.get<std::string>());
    }
}

void MessageParser::handle_modify(const nlohmann::json& msg) {
    try {
        // Ensure the message contains a valid "result" object
        if (msg.contains("result")) {
            auto result = msg["result"];
            if (result.contains("order")) {
                auto order = result["order"];

                // Extract relevant details from the order
                std::string order_id = order["order_id"];
                double price = order["price"];
                int amount = order["amount"];
                std::string instrument_name = order["instrument_name"];
                std::string order_state = order["order_state"];
                std::string direction = order["direction"];

                // Log the details for modification
                std::cout << "Modified Order Details: " << std::endl;
                std::cout << "Order ID: " << order_id << std::endl;
                std::cout << "Price: " << price << std::endl;
                std::cout << "Amount: " << amount << std::endl;
                std::cout << "Instrument: " << instrument_name << std::endl;
                std::cout << "Order State: " << order_state << std::endl;
                std::cout << "Direction: " << direction << std::endl;

                
            }
        } else {
            std::cerr << "Invalid response format: Missing 'result' field" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing modify response: " << e.what() << std::endl;
    }
}

void MessageParser:: handle_subscription_response(const nlohmann::json& msg) {
    try {
        // Extracting subscription details
        std::string channel = msg["params"]["channel"];
        auto& data = msg["params"]["data"];

        std::cout << "Channel: " << channel << std::endl;
        std::cout << "Instrument: " << data["instrument_name"] << std::endl;
        std::cout << "Change ID: " << data["change_id"] << std::endl;
        std::cout << "Previous Change ID: " << data["prev_change_id"] << std::endl;
        std::cout << "Timestamp: " << data["timestamp"] << std::endl;
        std::cout << "Type: " << data["type"] << std::endl;

        // Print asks if available
        if (data.contains("asks") && !data["asks"].empty()) {
            std::cout << "Asks: " << std::endl;
            for (const auto& ask : data["asks"]) {
                std::cout << "  - Price: " << ask[0] << ", Quantity: " << ask[1] << std::endl;
            }
        } else {
            std::cout << "Asks: No ask orders available." << std::endl;
        }

        // Print bids if available
        if (data.contains("bids") && !data["bids"].empty()) {
            std::cout << "Bids: " << std::endl;
            for (const auto& bid : data["bids"]) {
                std::string action = bid[0] == "new" ? "New" : "Deleted";
                std::cout << "  - Action: " << action << ", Price: " << bid[1] << ", Quantity: " << bid[2] << std::endl;
            }
        } else {
            std::cout << "Bids: No bid orders available." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error processing subscription message: " << e.what() << std::endl;
    }
}




// void MessageParser::parse_and_print(const std::string& raw_msg) {
//     try {
//         auto msg = json::parse(raw_msg);
        

//         // Basic Response Info
//         if (msg.contains("error")) {
//             std::cerr << "❌ Error: " << msg["error"].dump(2) << "\n";
//             return;
//         }

//         if (msg.contains("result")) {
//             std::cout << "✅ Result:\n" << std::setw(2) << msg["result"] << "\n";
//         }

//         if (msg.contains("method")) {
//             std::string method = msg["method"];
//             std::cout << "📩 Event: " << method << "\n";

//             if (msg.contains("params")) {
//                 std::cout << "🔍 Params:\n" << std::setw(2) << msg["params"] << "\n";
//             }
//         }

//         if (msg.contains("id")) {
//             std::cout << "🆔 Message ID: " << msg["id"] << "\n";
//         }

//     } catch (const std::exception& e) {
//         std::cerr << "🚨 Failed to parse message: " << e.what() << "\n";
//     }
// }
