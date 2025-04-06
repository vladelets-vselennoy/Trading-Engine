// #include "deribit_client.hpp"
// #include <iostream>

// int main() {
//     std::string api_key, api_secret;
//     std::cout << "🔑 Enter API Key: ";
//     std::cin >> api_key;
//     std::cout << "🔒 Enter API Secret: ";
//     std::cin >> api_secret;

//     DeribitClient client("test.deribit.com", "443", "/ws/api/v2");
//     client.connect();
//     client.start_receiving();
//     client.send_auth(api_key, api_secret);

//     while (true) {
//         std::cout << "\n📟 MENU:\n"
//                   << "1. Authenticate\n"
//                   << "2. Subscribe to BTC-PERPETUAL\n"
//                   << "3. Place Limit Buy Order\n"
//                   << "0. Exit\n"
//                   << "👉 Choose option: ";
//         int choice;
//         std::cin >> choice;

//         if (choice == 0) {
//             client.close();
//             break;
//         } else if (choice == 1) {
//             client.send_auth(api_key, api_secret);
//         } else if (choice == 2) {
//             client.subscribe_book("BTC-PERPETUAL");
//         } else if (choice == 3) {
//             std::string symbol;
//             double amount, price;
//             std::cout << "🪙 Enter Instrument (e.g. BTC-PERPETUAL): ";
//             std::cin >> symbol;
//             std::cout << "📦 Enter Amount: ";
//             std::cin >> amount;
//             std::cout << "💰 Enter Limit Price: ";
//             std::cin >> price;
//             client.place_order(symbol, amount, price);
//         } else {
//             std::cout << "❌ Invalid option\n";
//         }
//     }

//     return 0;
// }


#include "deribit_client.hpp"
#include <iostream>
#include <string>

void print_menu() {
    std::cout << "\n==== Deribit WebSocket Client ====\n";
    // std::cout << "1. Connect\n";
    // std::cout << "2. Authenticate\n";
    std::cout << "3. Subscribe to orderbook\n";
    std::cout << "4. Place order\n";
    std::cout << "5. Modify order\n";
    std::cout << "6. Cancel order\n";
    std::cout << "7. Cancel all orders\n";
    std::cout << "8. Get orderbook\n";
    std::cout << "9. Get positions\n";
    std::cout << "0. Exit\n";
    std::cout << "==================================\n";
    std::cout << "Enter your choice: ";
}

int main() {
    std::string api_key, api_secret;
    std::cout << "🔑 Enter API Key: ";
    std::cin >> api_key;
    std::cout << "🔒 Enter API Secret: ";
    std::cin >> api_secret;

    // DeribitClient client("test.deribit.com", "443", "/ws/api/v2");
   
    std::string host = "test.deribit.com";
    std::string port = "443";
    std::string target = "/ws/api/v2";

    DeribitClient client(host, port, target);
     client.connect();
    client.start_receiving();
    client.send_auth(api_key, api_secret);

    bool running = true;

    while (running) {
        print_menu();
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // flush newline

        switch (choice) {
            // case 1: {
            //     client.connect();
            //     break;
            // }
            // case 2: {
            //     std::string key, secret;
            //     std::cout << "Enter API Key: ";
            //     std::getline(std::cin, key);
            //     std::cout << "Enter API Secret: ";
            //     std::getline(std::cin, secret);
            //     client.send_auth(key, secret);
            //     break;
            // }
            case 3: {
                std::string symbol;
                std::cout << "Enter symbol to subscribe (e.g., BTC-PERPETUAL): ";
                std::getline(std::cin, symbol);
                client.subscribe_book(symbol);
                break;
            }
            case 4: {
                std::string symbol;
                double amount, price;
                std::cout << "Enter symbol: ";
                std::getline(std::cin, symbol);
                std::cout << "Enter amount: ";
                std::cin >> amount;
                std::cout << "Enter price: ";
                std::cin >> price;
                std::cin.ignore();
                client.place_order(symbol, amount, std::nullopt, "limit", price);
                break;
            }
            case 5: {
                std::string order_id;
                double amount, price;
                std::cout << "Enter Order ID: ";
                std::getline(std::cin, order_id);
                std::cout << "Enter new amount: ";
                std::cin >> amount;
                std::cout << "Enter new price: ";
                std::cin >> price;
                std::cin.ignore();
                client.modify_order(order_id, amount, price);
                break;
            }
            case 6: {
                std::string order_id;
                std::cout << "Enter Order ID to cancel: ";
                std::getline(std::cin, order_id);
                client.cancel_order(order_id);
                break;
            }
            case 7: {
                client.cancel_all();
                break;
            }
            case 8: {
                std::string symbol;
                std::cout << "Enter symbol for orderbook: ";
                std::getline(std::cin, symbol);
                client.get_orderbook(symbol);
                break;
            }
            case 9: {
                std::string currency;
                std::cout << "Enter currency (e.g., BTC): ";
                std::getline(std::cin, currency);
                client.get_positions(currency);
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

    return 0;
}
