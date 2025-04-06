#pragma once
//message_parser.hpp
#include <string>
#include <iostream>
#include "deribit_client.hpp"


void get_place_order_prompt(DeribitClient& client) {
    std::string symbol;
    double amount, price, trigger_price;
    std::string type, time_in_force, label, trigger;
    bool post_only, reduce_only;

    // Get the required symbol and amount
    std::cout << "Enter symbol: ";
    std::getline(std::cin, symbol);
    if (symbol.empty()) {
        std::cout << "❌ Symbol is required. Please enter a valid symbol.\n";
        return;
    }

    std::cout << "Enter amount: ";
    std::cin >> amount;
    std::cout << "Enter price: ";
    std::cin >> price;
    std::cin.ignore();  // To clear the newline left by std::cin

    // Get additional optional parameters
    std::cout << "Enter order type (e.g., limit, market): ";
    std::getline(std::cin, type);

    std::cout << "Enter time in force (optional - press Enter to skip): ";
    std::getline(std::cin, time_in_force);

    std::cout << "Enter label (optional - press Enter to skip): ";
    std::getline(std::cin, label);

    std::cout << "Enter trigger price (optional - press Enter to skip): ";
    std::string trigger_price_str;
    std::getline(std::cin, trigger_price_str);
    if (!trigger_price_str.empty()) {
        trigger_price = std::stod(trigger_price_str);
    } else {
        trigger_price = 0.0;  // Default if no input
    }

    std::cout << "Enter trigger (optional - press Enter to skip): ";
    std::getline(std::cin, trigger);

    std::cout << "Post-only (1 for Yes, 0 for No): ";
    std::cin >> post_only;

    std::cout << "Reduce-only (1 for Yes, 0 for No): ";
    std::cin >> reduce_only;
    std::cin.ignore();  // Clear the newline after std::cin

    // Call the place_order function with all parameters
    client.place_order(
        symbol,
        amount,
        std::nullopt,  // Optional, contracts not needed for this example
        type,
        price,
        !time_in_force.empty() ? std::optional<std::string>(time_in_force) : std::nullopt,
        post_only,
        reduce_only,
        !label.empty() ? std::optional<std::string>(label) : std::nullopt,
        trigger_price != 0.0 ? std::optional<double>(trigger_price) : std::nullopt,
        !trigger.empty() ? std::optional<std::string>(trigger) : std::nullopt
    );

    std::cout << "Order placed for " << symbol << " with amount " << amount << " at price " << price << ".\n";
}

void get_modify_order_prompt(DeribitClient& client) {
    std::string order_id;
    double amount, price;
    std::string post_only, time_in_force, label;

    // Get the required order_id
    std::cout << "Enter Order ID to modify: ";
    std::getline(std::cin, order_id);
    if (order_id.empty()) {
        std::cout << "❌ Order ID is required. Please enter a valid Order ID.\n";
        return;
    }

    // Get the new amount and price
    std::cout << "Enter new amount: ";
    std::cin >> amount;
    std::cout << "Enter new price: ";
    std::cin >> price;
    std::cin.ignore();  // To clear the newline left by std::cin

    // Get optional parameters for modification
    std::cout << "Enter new post-only (1 for Yes, 0 for No, press Enter to skip): ";
    std::getline(std::cin, post_only);
    
    std::cout << "Enter new time in force (optional - press Enter to skip): ";
    std::getline(std::cin, time_in_force);

    std::cout << "Enter new label (optional - press Enter to skip): ";
    std::getline(std::cin, label);

    // Call the modify_order function with all parameters
    client.modify_order(
        order_id,
        amount,
        price,
        !post_only.empty() ? std::optional<std::string>(post_only) : std::nullopt,
        !time_in_force.empty() ? std::optional<std::string>(time_in_force) : std::nullopt,
        !label.empty() ? std::optional<std::string>(label) : std::nullopt
    );

    std::cout << "Order " << order_id << " modified with new amount " << amount << " and price " << price << ".\n";
}

// Function to handle the prompt and fetching the orderbook
void get_orderbook_prompt(DeribitClient& client) {
    std::string symbol;
    int depth;

    // Get the symbol for the orderbook
    std::cout << "Enter symbol for orderbook (e.g., BTC-PERPETUAL): ";
    std::getline(std::cin, symbol);
    if (symbol.empty()) {
        std::cout << "Symbol is required. Please enter a valid symbol.\n";
        return;
    }

    // Get the depth for the orderbook
    std::cout << "Enter depth for orderbook (default is 10, press Enter to skip): ";
    std::string depth_input;
    std::getline(std::cin, depth_input);
    
    // If the user provided a value for depth, convert it to an integer
    if (!depth_input.empty()) {
        try {
            depth = std::stoi(depth_input);
        } catch (const std::invalid_argument&) {
            std::cout << "Invalid input for depth. Using default depth of 10.\n";
            depth = 10;
        }
    } else {
        depth = 10;  // Default depth
    }

    // Call the get_orderbook function with the user-provided symbol and depth
    client.get_orderbook(symbol, depth);

    std::cout << "Orderbook for symbol " << symbol << " with depth " << depth << " requested.\n";
}

// Function to handle the prompt and fetching positions
void get_positions_prompt(DeribitClient& client) {
    std::string currency;
    std::string kind;

    // Get the currency for the positions
    std::cout << "Enter currency for positions (e.g., BTC): ";
    std::getline(std::cin, currency);
    if (currency.empty()) {
        std::cout << "Currency is required. Please enter a valid currency.\n";
        return;
    }

    // Get the kind of positions (optional)
    std::cout << "Enter kind for positions (optional, press Enter to skip, default is 'any'): ";
    std::getline(std::cin, kind);
    if (kind.empty()) {
        kind = "any";  // Default to "any" if the user does not specify
    }

    // Call the get_positions function with the user-provided currency and kind
    client.get_positions(currency, kind);

    std::cout << "Positions for currency " << currency << " and kind " << kind << " requested.\n";
}

// Function to handle the prompt for unsubscribing from a symbol
void get_unsubscribe_prompt(DeribitClient& client) {
    std::string symbol;

    // Get the symbol the user wants to unsubscribe from
    std::cout << "Enter symbol to unsubscribe from (e.g., BTC-PERPETUAL): ";
    std::getline(std::cin, symbol);
    
    if (symbol.empty()) {
        std::cout << "Symbol is required. Please enter a valid symbol.\n";
        return;
    }

    // Call the unsubscribe function with the user-provided symbol
    client.unsubscribe(symbol);

    std::cout << "Unsubscribed from symbol: " << symbol << "\n";
}
