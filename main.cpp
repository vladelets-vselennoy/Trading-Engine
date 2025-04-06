#include "deribit_client.hpp"
#include <iostream>
#include <string>
#include <prompts.hpp>  

/// @brief Displays the main menu options to the user
/// @details Shows available operations including:
///          - Authentication
///          - List subscriptions
///          - Order operations (place, modify, cancel)
///          - Market data operations (orderbook, positions)
void print_menu() {
    std::cout << "\n==== Deribit WebSocket Client ====\n";
    std::cout << "1. Authenticate\n";
    std::cout << "2. List Subscriptions\n";
    std::cout << "3. Place order\n";
    std::cout << "4. Modify order\n";
    std::cout << "5. Cancel order\n";
    std::cout << "6. Cancel all orders\n";
    std::cout << "7. Get orderbook\n";
    std::cout << "8. Get positions\n";
    std::cout << "9. Subscribe to orderbook\n";
    std::cout << "0. Exit\n";
    std::cout << "==================================\n";
    std::cout << "Enter your choice: ";
}

/// @brief Main entry point for the Deribit WebSocket client
/// @details Initializes the client, handles the main menu loop,
///          and provides error handling for the application
/// @return 0 on successful execution, non-zero on error
int main() {
    std::string api_key, api_secret;
    std::string host = "test.deribit.com";
    std::string port = "443";
    std::string target = "/ws/api/v2";

    try {
        // Initialize logging
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l: %v");
        spdlog::flush_on(spdlog::level::debug);  
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("📡 Starting Deribit WebSocket Client...");

        // Create and connect client
        DeribitClient client(host, port, target);
        client.connect();

        // Main menu loop
        bool running = true;
        while (running) {
            print_menu();
            int choice;
            std::cin >> choice;
            std::cin.ignore(); // flush newline

            switch (choice) {
                case 1: {
                    std::cout << "Enter API Key: ";
                    std::cin >> api_key;
                    std::cout << "Enter API Secret: ";
                    std::cin >> api_secret;

                    client.send_auth(api_key, api_secret);
                    break;
                }
                case 2: {
                    client.list_subscriptions();
                    break;
                }
                case 3: {
                    get_place_order_prompt(client);
                    break;
                }
                case 4: {
                    get_modify_order_prompt(client);
                    break;
                }
                case 5: {
                    std::string order_id;
                    std::cout << "Enter Order ID to cancel: ";
                    std::getline(std::cin, order_id);
                    client.cancel_order(order_id);
                    break;
                }
                case 6: {
                    client.cancel_all();
                    break;
                }
                case 7: {
                    get_orderbook_prompt(client);
                    break;
                }
                case 8: {
                    get_positions_prompt(client);
                    break;
                }
                case 9: {
                    std::string symbol;
                    std::cout << "Enter symbol to subscribe (e.g., BTC-PERPETUAL): ";
                    std::getline(std::cin, symbol);
                    client.subscribe_book(symbol);
                    break;
                }
                case 0: {
                    running = false;
                    client.close();
                    break;
                }
                default:
                    std::cout << "❌ Invalid choice. Try again.\n";
                    break;
            }
        }
    } 
    catch (const std::exception& e) {
        spdlog::error("💥 Unhandled exception in main: {}", e.what());
        return 1;
    } 
    catch (...) {
        spdlog::error("💥 Unknown unhandled exception in main");
        return 2;
    }

    return 0;
}