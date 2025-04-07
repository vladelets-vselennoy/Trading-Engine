# Deribit API Client

A high-performance C++ WebSocket client for the Deribit cryptocurrency exchange API.

## Features

- Real-time WebSocket communication
- Authentication and secure connections via TLS/SSL
- Comprehensive order management (create, modify, cancel)
- Position tracking and real-time market data
- Thread-safe message handling
- Local WebSocket server for client applications
- Detailed logging system

## Requirements

- C++17 compiler (GCC 7+, Clang 5+)
- CMake 3.16+
- Boost libraries (System, Thread, Asio, Beast)
- OpenSSL
- nlohmann/json
- spdlog
- fmt

## Installation

### Manual Build
```bash
mkdir build && cd build
cmake ..
make
```

### Using the Build Script
The project includes a convenient build script that handles the build process and can run the application:

```bash
# Build and run with default settings (Release build)
./build_and_run.sh

# Clean build and run with debug configuration
./build_and_run.sh --clean --build-type Debug

# Build with 4 parallel jobs
./build_and_run.sh --jobs 4

# Pass arguments to the executable
./build_and_run.sh -- --verbose
```

#### Build Script Options
- `--clean`: Remove previous build directory before building
- `--build-type TYPE`: Set CMake build type (Debug, Release, RelWithDebInfo, MinSizeRel)
- `--jobs N`: Number of parallel jobs for make
- `--help`: Display help message
- `--`: Arguments after this are passed to the executable

## Quick Start

```cpp
#include "deribit_client.hpp"

int main() {
    DeribitClient client("test.deribit.com", "443", "/ws/api/v2", 8080);
    client.connect();
    
    // Authenticate
    client.send_auth("your_api_key", "your_api_secret");
    
    // Subscribe to orderbook
    client.subscribe_book("BTC-PERPETUAL");
    
    // Place an order
    client.place_order("BTC-PERPETUAL", 0.1, std::nullopt, "limit", 50000);
}
```

## Core Components

### DeribitClient
- Main interface for API interactions
- Manages WebSocket connections and message routing
- Handles authentication and session management

### MessageParser
- Processes incoming JSON messages
- Routes messages to appropriate handlers
- Provides structured data to client applications

### WebSocket Server
- Local WebSocket server for client applications
- Supports multiple client connections
- Real-time data broadcasting

## Configuration

Environment variables:
```bash
export DERIBIT_API_KEY="your_api_key"
export DERIBIT_API_SECRET="your_api_secret"
export DERIBIT_TESTNET="1"  # For testnet
```

## API Reference

### Authentication
```cpp
client.send_auth(api_key, api_secret);
```

### Order Management
```cpp
// Place a new order
client.place_order("BTC-PERPETUAL", 0.1, std::nullopt, "limit", 50000);

// Modify an existing order
client.modify_order("order_id_here", 0.2, 49000);

// Cancel an order
client.cancel_order("order_id_here");

// Cancel all orders
client.cancel_all();
```

### Market Data
```cpp
// Get orderbook (depth = 10)
client.get_orderbook("BTC-PERPETUAL", 10);

// Subscribe to orderbook updates
client.subscribe_book("BTC-PERPETUAL");

// Unsubscribe from orderbook updates
client.unsubscribe("BTC-PERPETUAL");

// Get positions
client.get_positions("BTC");
```

## Error Handling

The client includes comprehensive error handling:
- Connection failures
- Authentication errors
- Message parsing errors
- Network timeouts
- Invalid responses

## Logging

The client uses spdlog for structured logging:
- File logging with rotation
- Console output for critical events
- Different severity levels (debug, info, warning, error)
- Custom formatting

Example log configuration:
```cpp
spdlog::set_level(spdlog::level::debug);
spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l: %v");
```

## Thread Safety

The client uses multiple threads for efficient operation:
- Dedicated thread for receiving messages
- Dedicated thread for sending messages
- Dedicated thread for processing received messages
- Thread-safe queues for message passing
- Mutex protection for shared resources

## WebSocket Server

The client includes a local WebSocket server to:
- Provide data to local client applications
- Support multiple simultaneous connections
- Manage subscriptions per client
- Broadcast real-time updates

Starting the server:
```cpp
client.start_server();  // Default port is specified in constructor
```

## Example Use Cases

### Algorithmic Trading
- Receive real-time market data
- Execute trading strategies based on market conditions
- Monitor positions and portfolio risk

### Market Making
- Maintain bid/ask orders
- Update quotes based on market movements
- Monitor execution and adjust strategies

### Data Collection
- Subscribe to market data
- Store information for later analysis
- Track historical price movements

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Support

For issues and feature requests, please use the GitHub issue tracker.
