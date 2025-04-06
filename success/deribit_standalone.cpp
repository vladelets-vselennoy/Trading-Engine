#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <mutex>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using json = nlohmann::json;

std::mutex mtx;

void fail(beast::error_code ec, const char* what) {
    std::cerr << what << ": " << ec.message() << "\n";
    exit(1);
}

void receive_messages(websocket::stream<beast::ssl_stream<tcp::socket>>& ws) {
    try {
        beast::flat_buffer buffer;
        while (true) {
            ws.read(buffer);
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "📩 " << beast::buffers_to_string(buffer.data()) << "\n";
            buffer.clear();
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Read Error: " << e.what() << "\n";
    }
}

int main() {
    std::string api_key, api_secret;
    std::cout << "🔑 Enter API Key: ";
    std::cin >> api_key;
    std::cout << "🔒 Enter API Secret: ";
    std::cin >> api_secret;

    const std::string host = "test.deribit.com";
    const std::string port = "443";
    const std::string target = "/ws/api/v2";

    boost::asio::io_context ioc;
    ssl::context ctx(ssl::context::tlsv12_client);
    ctx.set_default_verify_paths();

    websocket::stream<beast::ssl_stream<tcp::socket>> ws(ioc, ctx);

    tcp::resolver resolver(ioc);
    auto const results = resolver.resolve(host, port);

    boost::asio::connect(beast::get_lowest_layer(ws), results.begin(), results.end());
    ws.next_layer().handshake(ssl::stream_base::client);
    ws.handshake(host, target);

    std::cout << " Connected to Deribit WebSocket\n";

    std::thread recv_thread(receive_messages, std::ref(ws));

    while (true) {
        std::cout << "\n MENU:\n"
                  << "1. Authenticate (private/login)\n"
                  << "2. Subscribe to BTC-PERPETUAL Order Book\n"
                  << "3. Place Limit Buy Order (testnet)\n"
                  << "0. Exit\n"
                  << " Choose an option: ";

        int choice;
        std::cin >> choice;

        json msg;

        if (choice == 0) {
            ws.close(websocket::close_code::normal);
            break;
        } else if (choice == 1) {
            msg = {
                {"jsonrpc", "2.0"},
                {"id", 1},
                {"method", "public/auth"},
                {"params", {
                    {"grant_type", "client_credentials"},
                    {"client_id", api_key},
                    {"client_secret", api_secret}
                }}
            };
        } else if (choice == 2) {
            msg = {
                {"jsonrpc", "2.0"},
                {"id", 2},
                {"method", "public/subscribe"},
                {"params", {
                    {"channels", {"book.BTC-PERPETUAL.100ms"}}
                }}
            };
        } else if (choice == 3) {
            msg = {
                {"jsonrpc", "2.0"},
                {"id", 3},
                {"method", "private/buy"},
                {"params", {
                    {"instrument_name", "BTC-PERPETUAL"},
                    {"amount", 10},
                    {"type", "limit"},
                    {"price", 1000.0},
                    {"post_only", true}
                }}
            };
        } else {
            std::cout << "❌ Invalid option. Try again.\n";
            continue;
        }

        std::lock_guard<std::mutex> lock(mtx);
        ws.write(boost::asio::buffer(msg.dump()));
    }

    recv_thread.join();
    std::cout << "👋 Disconnected.\n";
    return 0;
}
