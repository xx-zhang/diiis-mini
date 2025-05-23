# 重构代码为C++并添加调试模式
_Exported on 2025/5/23 at GMT+8 16:48:34 from Cursor (0.50.5)_

---

**User**

我需要重构这个代码，请帮我直接重构成c++,  另外增加上debug模式，所有的操作（例如登录、 读取数据）等等都需要给出详细的上下文。 请用cpp 分模块帮我进行完善这个server。 
@https://github.com/blizzless/blizzless-diiis 

另外为了方便您进行快速开发，请通过这个连接来学习下这个里面的内容。@https://deepwiki.com/blizzless/blizzless-diiis  ，这个里面涉及到数据库是pgsql， 修改为sqlite, 另外这个项目里面的单元测试我们需要有； 因为 后面我还需要实际上线进行操作。

请仔细查看上下文完善，帮我把代码和相关说明细节，还有开发文档等等都记录清楚，记录到 docs目录中。还有GM操作，创建账号，帮我封装到 rest里面，让我直接curl就能创建

---

**Cursor**

I'll help you restructure this Diablo 3 server implementation from C# to C++, implement debug mode, use SQLite instead of PostgreSQL, add unit testing, and create proper documentation.

Let's start by exploring the repository structure to better understand the codebase.

Let's look at the project structure to understand how it's organized.

Let me check for source files in case it's in a subdirectory.

Let's create a project structure for the new C++ implementation:

First, let's create our base project files: the CMakeLists.txt file for building the project:

```plaintext
cmake_minimum_required(VERSION 3.12)
project(D3Server VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build the test suite" ON)
option(DEBUG_MODE "Enable debug mode" ON)

# Dependencies
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Define debug mode
if(DEBUG_MODE)
    add_definitions(-DDEBUG_MODE)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SQLite3_INCLUDE_DIRS}
)

# Add subdirectories
add_subdirectory(src)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
```

Let's create the CMake file for the source directory:

```plaintext
add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(database)
add_subdirectory(battle_net)
add_subdirectory(game_server)
add_subdirectory(rest_api)

# Main executable
add_executable(d3server main.cpp)

target_link_libraries(d3server 
    PRIVATE
    core
    utils
    database
    battle_net
    game_server
    rest_api
    ${SQLite3_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

# Installation
install(TARGETS d3server
    RUNTIME DESTINATION bin
)
```

Let's create CMakeLists.txt files for each module:

```plaintext
add_library(core
    src/server.cpp
    src/config.cpp
    src/logger.cpp
)

target_include_directories(core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core
    PRIVATE
    utils
    ${OPENSSL_LIBRARIES}
)
```

Let's create similar directory structures for the other modules:

Now let's create the CMakeLists.txt files for the other modules:

```plaintext
add_library(utils
    src/string_utils.cpp
    src/file_utils.cpp
    src/crypto_utils.cpp
    src/debug.cpp
)

target_include_directories(utils
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(utils
    PRIVATE
    ${OPENSSL_LIBRARIES}
)
```

```plaintext
add_library(database
    src/database_manager.cpp
    src/account_dao.cpp
    src/character_dao.cpp
    src/world_dao.cpp
)

target_include_directories(database
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(database
    PRIVATE
    utils
    ${SQLite3_LIBRARIES}
)
```

```plaintext
add_library(battle_net
    src/battle_net_server.cpp
    src/auth_server.cpp
    src/protocol.cpp
    src/session.cpp
)

target_include_directories(battle_net
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(battle_net
    PRIVATE
    core
    utils
    database
)
```

```plaintext
add_library(game_server
    src/game_server.cpp
    src/world.cpp
    src/player.cpp
    src/monster.cpp
    src/item.cpp
    src/quest.cpp
    src/chat.cpp
    src/ai.cpp
    src/command_handler.cpp
)

target_include_directories(game_server
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(game_server
    PRIVATE
    core
    utils
    database
)
```

```plaintext
add_library(rest_api
    src/rest_server.cpp
    src/account_controller.cpp
    src/admin_controller.cpp
    src/http_utils.cpp
)

target_include_directories(rest_api
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rest_api
    PRIVATE
    core
    utils
    database
)
```

Let's create the main.cpp file:

```cpp
#include <iostream>
#include <string>
#include <memory>
#include <csignal>
#include <thread>

#include "core/server.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"
#include "battle_net/battle_net_server.h"
#include "game_server/game_server.h"
#include "rest_api/rest_server.h"

// Global server instance for signal handling
std::shared_ptr<d3server::core::Server> g_server;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::INFO, 
        "Received signal " + std::to_string(signal) + ", shutting down...");
    
    if (g_server) {
        g_server->shutdown();
    }
}

int main(int argc, char* argv[]) {
    // Set up signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // Initialize logger
        auto& logger = d3server::core::Logger::getInstance();
        logger.init();
        logger.log(d3server::core::LogLevel::INFO, "D3Server starting up...");
        
        DEBUG_LOG("Debug mode is enabled");
        
        // Load configuration
        auto config = std::make_shared<d3server::core::Config>();
        config->loadFromFile("config.ini");
        logger.log(d3server::core::LogLevel::INFO, "Configuration loaded");
        
        // Initialize database
        auto dbManager = std::make_shared<d3server::database::DatabaseManager>();
        dbManager->init(config->getDatabaseConfig());
        logger.log(d3server::core::LogLevel::INFO, "Database initialized");
        
        // Initialize server components
        auto battleNetServer = std::make_shared<d3server::battle_net::BattleNetServer>(config, dbManager);
        auto gameServer = std::make_shared<d3server::game_server::GameServer>(config, dbManager);
        auto restServer = std::make_shared<d3server::rest_api::RestServer>(config, dbManager);
        
        // Create main server instance
        g_server = std::make_shared<d3server::core::Server>(
            config, 
            dbManager, 
            battleNetServer,
            gameServer,
            restServer
        );
        
        // Initialize and start the server
        g_server->init();
        g_server->run();
        
        logger.log(d3server::core::LogLevel::INFO, "Server shutdown complete");
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
```

Let's create the CMakeLists.txt file for the tests:

```plaintext
find_package(GTest REQUIRED)

add_subdirectory(unit)
```

```plaintext
include(GoogleTest)

# Core module tests
add_executable(core_tests
    core_tests.cpp
    config_tests.cpp
    logger_tests.cpp
)

target_link_libraries(core_tests
    PRIVATE
    core
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(core_tests)

# Database module tests
add_executable(database_tests
    database_tests.cpp
    account_dao_tests.cpp
    character_dao_tests.cpp
)

target_link_libraries(database_tests
    PRIVATE
    database
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(database_tests)

# Battle.net module tests
add_executable(battle_net_tests
    battle_net_tests.cpp
    auth_tests.cpp
)

target_link_libraries(battle_net_tests
    PRIVATE
    battle_net
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(battle_net_tests)

# Game server module tests
add_executable(game_server_tests
    game_server_tests.cpp
    world_tests.cpp
    player_tests.cpp
)

target_link_libraries(game_server_tests
    PRIVATE
    game_server
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(game_server_tests)

# REST API module tests
add_executable(rest_api_tests
    rest_api_tests.cpp
    account_controller_tests.cpp
)

target_link_libraries(rest_api_tests
    PRIVATE
    rest_api
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(rest_api_tests)
```

Now let's set up some basic documentation:

```markdown
# D3Server Documentation

This is the main documentation for the D3Server project, a C++ implementation of a Diablo III server.

## Table of Contents

1. [Overview](overview.md)
2. [Installation](installation.md)
3. [Architecture](architecture.md)
4. [Configuration](configuration.md)
5. [Database](database.md)
6. [Battle.net Server](battle_net.md)
7. [Game Server](game_server.md)
8. [REST API](rest_api.md)
9. [GM Commands](gm_commands.md)
10. [Debugging](debugging.md)
11. [Testing](testing.md)
12. [Contributing](contributing.md)

## Quick Start

See the [Installation](installation.md) guide to get started quickly.

## Project Goals

- Provide a fully functional Diablo III server implementation
- Support standard gameplay features
- Include administration tools and GM commands
- Provide detailed debugging information
- Modular architecture for easy extension
```

Let's create a basic installation documentation:

```markdown
# Installation Guide

This guide will help you set up the D3Server on your system.

## System Requirements

- **CPU**: Intel Core i5 or AMD Ryzen 5 (minimum), Intel Core i7 or AMD Ryzen 7 (recommended)
- **Memory**: 4 GB RAM (minimum), 16 GB RAM (recommended)
- **Disk Space**: 1 GB

## Prerequisites

The following software needs to be installed on your system:

- C++ compiler with C++17 support (GCC 8+ or Clang 7+)
- CMake 3.12 or higher
- SQLite3 development libraries
- OpenSSL development libraries
- Google Test (for running tests)

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libsqlite3-dev libssl-dev libgtest-dev
```

### CentOS/RHEL

```bash
sudo yum install -y gcc-c++ cmake sqlite-devel openssl-devel gtest-devel
```

### macOS

```bash
brew install cmake sqlite openssl googletest
```

## Building from Source

1. Clone the repository:

```bash
git clone https://github.com/your-username/d3server.git
cd d3server
```

2. Create a build directory:

```bash
mkdir build
cd build
```

3. Configure with CMake:

```bash
cmake ..
```

4. Build the server:

```bash
make
```

5. Run tests (optional):

```bash
make test
```

6. Install (optional):

```bash
sudo make install
```

## Configuration

Copy the example configuration file to the default location:

```bash
cp ../configs/config.ini.example ./config.ini
```

Edit the configuration file according to your needs. See the [Configuration](configuration.md) documentation for details.

## Database Initialization

The server will automatically create the SQLite database on first run. However, you can initialize it manually:

```bash
./d3server --init-db
```

## Running the Server

```bash
./d3server
```

By default, the server will run in the foreground. Use the `--daemon` option to run it in the background.

## Creating an Account

Use the REST API to create an account:

```bash
curl -X POST "http://localhost:8080/api/accounts" \
  -H "Content-Type: application/json" \
  -d '{"login":"username@", "password":"YourPassword", "battleTag":"YourBattleTag"}'
```

Or to create an account with admin privileges:

```bash
curl -X POST "http://localhost:8080/api/accounts" \
  -H "Content-Type: application/json" \
  -d '{"login":"username@", "password":"YourPassword", "battleTag":"YourBattleTag", "role":"admin"}'
```

See [GM Commands](gm_commands.md) for more administrative operations.

## Client Configuration

1. Install the certificate from the `certs` directory: `bnetserver.p12` (password: `123`)

2. Modify hosts file or the game client to redirect to your server:

   ### Method 1: Hosts File
   
   Add to your hosts file (`C:\Windows\System32\drivers\etc\hosts` on Windows, `/etc/hosts` on Linux):
   
   ```
   127.0.0.1 us.actual.battle.net
   127.0.0.1 eu.actual.battle.net
   ```
   
   ### Method 2: Modify Game Executable
   
   Edit the game's executable file with a hex editor to replace the server addresses with your own.

3. Launch the client with:

   ```
   Diablo III64.exe -launch
   ```

## Troubleshooting

See the [Debugging](debugging.md) guide for troubleshooting information.
```

Let's create documentation for GM commands:

```markdown
# GM Commands

This document outlines the available Game Master (GM) commands for server administration.

## Using the GM Command System

There are two ways to use GM commands:

1. **In-game commands**: Used by admins while connected to the game
2. **REST API endpoints**: Used for external administration

## Authentication

For in-game commands, you must be logged in with an account that has appropriate admin privileges.

For REST API commands, you must authenticate using an admin API key or credentials.

## Available Commands

### Account Management

#### Create Account

**In-game:**
```
!account add <login> <password> <battleTag> [role]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/accounts" \
  -H "Content-Type: application/json" \
  -d '{
    "login": "username@",
    "password": "YourPassword",
    "battleTag": "YourBattleTag",
    "role": "user"
  }'
```

Roles: `user`, `gm`, `admin`, `owner`

#### Delete Account

**In-game:**
```
!account delete <login>
```

**REST API:**
```bash
curl -X DELETE "http://localhost:8080/api/accounts/<login>" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

#### Ban Account

**In-game:**
```
!account ban <login> <reason> [duration]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/accounts/<login>/ban" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "reason": "Violation of rules",
    "duration": "7d"
  }'
```

Duration format: `[number][unit]` where unit can be `m` (minutes), `h` (hours), `d` (days)

#### List Accounts

**In-game:**
```
!account list [page] [limit]
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/accounts?page=1&limit=20" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

### Character Management

#### Create Character

**In-game:**
```
!character create <accountLogin> <characterName> <class> <level>
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/accounts/<login>/characters" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "name": "CharacterName",
    "class": "barbarian",
    "level": 70
  }'
```

Classes: `barbarian`, `crusader`, `demon_hunter`, `monk`, `witch_doctor`, `wizard`, `necromancer`

#### Delete Character

**In-game:**
```
!character delete <characterId>
```

**REST API:**
```bash
curl -X DELETE "http://localhost:8080/api/characters/<characterId>" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

#### Modify Character

**In-game:**
```
!character modify <characterId> <property> <value>
```

**REST API:**
```bash
curl -X PATCH "http://localhost:8080/api/characters/<characterId>" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "property": "level",
    "value": 70
  }'
```

### World Management

#### Restart World

**In-game:**
```
!world restart <worldId>
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/worlds/<worldId>/restart" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

#### List Worlds

**In-game:**
```
!world list
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/worlds" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

### Item Management

#### Give Item

**In-game:**
```
!item give <characterId> <itemId> [quality] [quantity]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/characters/<characterId>/items" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "itemId": "Unique_Sword_2H_001_x1",
    "quality": "legendary",
    "quantity": 1
  }'
```

### Server Management

#### Shutdown Server

**In-game:**
```
!server shutdown [delay]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/server/shutdown" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "delay": "5m"
  }'
```

#### Restart Server

**In-game:**
```
!server restart [delay]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/server/restart" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "delay": "5m"
  }'
```

#### Server Status

**In-game:**
```
!server status
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/server/status" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

## Logging

All GM commands are logged for security purposes. Logs can be accessed:

**In-game:**
```
!logs view [lines]
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/logs?lines=100" \
  -H "Authorization: Bearer YOUR_API_KEY"
```
```

Now let's implement the debug utility:

```cpp
#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <functional>

namespace d3server {
namespace utils {

/**
 * @brief Debug utility class for enhanced debugging capabilities
 */
class Debug {
public:
    /**
     * @brief Get the singleton instance of the Debug class
     * @return Reference to the Debug instance
     */
    static Debug& getInstance();

    /**
     * @brief Initialize debug system
     * @param enableDebug Whether debug mode is enabled
     */
    void init(bool enableDebug = true);

    /**
     * @brief Log a debug message if debug mode is enabled
     * @param file Source file name
     * @param line Source line number
     * @param function Function name
     * @param message Debug message
     */
    void log(const std::string& file, int line, const std::string& function, const std::string& message);

    /**
     * @brief Start a timer for performance measurement
     * @param operationName Name of the operation being timed
     * @return Timer ID for stopping the timer
     */
    int startTimer(const std::string& operationName);

    /**
     * @brief Stop a timer and log the elapsed time
     * @param timerId Timer ID returned by startTimer
     * @param file Source file name
     * @param line Source line number
     * @param function Function name
     */
    void stopTimer(int timerId, const std::string& file, int line, const std::string& function);

    /**
     * @brief Set a callback to be called when debug messages are logged
     * @param callback The callback function
     */
    void setLogCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Check if debug mode is enabled
     * @return True if debug is enabled, false otherwise
     */
    bool isDebugEnabled() const;

private:
    Debug(); // Private constructor for singleton
    ~Debug() = default;
    Debug(const Debug&) = delete;
    Debug& operator=(const Debug&) = delete;

    bool m_debugEnabled;
    std::mutex m_mutex;
    std::function<void(const std::string&)> m_logCallback;

    struct Timer {
        std::string operationName;
        std::chrono::high_resolution_clock::time_point startTime;
    };

    std::vector<Timer> m_timers;
};

// Convenience macros for debug logging
#ifdef DEBUG_MODE
#define DEBUG_LOG(message) \
    d3server::utils::Debug::getInstance().log(__FILE__, __LINE__, __FUNCTION__, message)

#define DEBUG_TIMER_START(operation) \
    int timerId = d3server::utils::Debug::getInstance().startTimer(operation)

#define DEBUG_TIMER_STOP() \
    d3server::utils::Debug::getInstance().stopTimer(timerId, __FILE__, __LINE__, __FUNCTION__)

#define DEBUG_FUNCTION_ENTER() \
    d3server::utils::Debug::getInstance().log(__FILE__, __LINE__, __FUNCTION__, "Function entered")

#define DEBUG_FUNCTION_EXIT() \
    d3server::utils::Debug::getInstance().log(__FILE__, __LINE__, __FUNCTION__, "Function exited")

#define DEBUG_CONTEXT(ctx) \
    d3server::utils::Debug::getInstance().log(__FILE__, __LINE__, __FUNCTION__, "Context: " + std::string(ctx))

#define DEBUG_VARIABLE(var) \
    { \
        std::stringstream ss; \
        ss << #var << " = " << var; \
        d3server::utils::Debug::getInstance().log(__FILE__, __LINE__, __FUNCTION__, ss.str()); \
    }
#else
#define DEBUG_LOG(message) ((void)0)
#define DEBUG_TIMER_START(operation) ((void)0)
#define DEBUG_TIMER_STOP() ((void)0)
#define DEBUG_FUNCTION_ENTER() ((void)0)
#define DEBUG_FUNCTION_EXIT() ((void)0)
#define DEBUG_CONTEXT(ctx) ((void)0)
#define DEBUG_VARIABLE(var) ((void)0)
#endif

} // namespace utils
} // namespace d3server
```

Let's implement the debug class:

```cpp
#include "utils/debug.h"
#include <filesystem>
#include <ctime>

namespace d3server {
namespace utils {

Debug::Debug() : m_debugEnabled(false) {}

Debug& Debug::getInstance() {
    static Debug instance;
    return instance;
}

void Debug::init(bool enableDebug) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_debugEnabled = enableDebug;
    if (m_debugEnabled) {
        log(__FILE__, __LINE__, __FUNCTION__, "Debug mode initialized");
    }
}

void Debug::log(const std::string& file, int line, const std::string& function, const std::string& message) {
    if (!m_debugEnabled) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << now_ms.count();

    // Extract file name only
    std::filesystem::path filePath(file);
    std::string fileName = filePath.filename().string();
    
    // Get thread ID
    std::stringstream threadId;
    threadId << std::this_thread::get_id();

    // Format debug message
    std::string formattedMessage = 
        ss.str() + " [" + threadId.str() + "] " + 
        fileName + ":" + std::to_string(line) + 
        " (" + function + ") " + message;
    
    // Output to console
    std::cout << "[DEBUG] " << formattedMessage << std::endl;
    
    // Call callback if set
    if (m_logCallback) {
        m_logCallback(formattedMessage);
    }
}

int Debug::startTimer(const std::string& operationName) {
    if (!m_debugEnabled) {
        return -1;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Create a new timer
    Timer timer;
    timer.operationName = operationName;
    timer.startTime = std::chrono::high_resolution_clock::now();
    
    // Add to timer vector and return its index
    m_timers.push_back(timer);
    return static_cast<int>(m_timers.size() - 1);
}

void Debug::stopTimer(int timerId, const std::string& file, int line, const std::string& function) {
    if (!m_debugEnabled || timerId < 0 || static_cast<size_t>(timerId) >= m_timers.size()) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Get elapsed time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto startTime = m_timers[timerId].startTime;
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    // Format message
    std::stringstream message;
    message << "Operation '" << m_timers[timerId].operationName 
            << "' completed in " << elapsed.count() 
            << " μs (" << (elapsed.count() / 1000.0) << " ms)";
    
    // Log the timing information
    log(file, line, function, message.str());
}

void Debug::setLogCallback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logCallback = callback;
}

bool Debug::isDebugEnabled() const {
    return m_debugEnabled;
}

} // namespace utils
} // namespace d3server
```

Let's implement the logger:

```cpp
#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <functional>

namespace d3server {
namespace core {

/**
 * @brief Log levels for the logging system
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief Main logging class for the server
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of the Logger
     * @return Reference to the Logger instance
     */
    static Logger& getInstance();

    /**
     * @brief Initialize the logger
     * @param logFilePath Path to the log file (default: "d3server.log")
     * @return True if initialized successfully, false otherwise
     */
    bool init(const std::string& logFilePath = "d3server.log");

    /**
     * @brief Log a message with the specified log level
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Set minimum log level to display
     * @param level Minimum log level
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Set whether to log to console
     * @param logToConsole True to log to console, false otherwise
     */
    void setLogToConsole(bool logToConsole);

    /**
     * @brief Register a log callback function
     * @param callback Function to call for each log message
     */
    void registerCallback(std::function<void(LogLevel, const std::string&)> callback);

    /**
     * @brief Close the logger
     */
    void close();

    /**
     * @brief Get log level as string
     * @param level Log level
     * @return String representation of log level
     */
    static std::string logLevelToString(LogLevel level);

private:
    Logger(); // Private constructor for singleton
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex m_mutex;
    std::ofstream m_logFile;
    LogLevel m_minLevel;
    bool m_logToConsole;
    std::vector<std::function<void(LogLevel, const std::string&)>> m_callbacks;
    bool m_initialized;
};

// Convenience macros for logging
#define LOG_DEBUG(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::DEBUG, message)

#define LOG_INFO(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::INFO, message)

#define LOG_WARNING(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::WARNING, message)

#define LOG_ERROR(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::ERROR, message)

#define LOG_CRITICAL(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::CRITICAL, message)

} // namespace core
} // namespace d3server
```

The model's attempt to create the file did not work correctly. Let me try again:

```cpp
#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <functional>

namespace d3server {
namespace core {

/**
 * @brief Log levels for the logging system
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief Main logging class for the server
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of the Logger
     * @return Reference to the Logger instance
     */
    static Logger& getInstance();

    /**
     * @brief Initialize the logger
     * @param logFilePath Path to the log file (default: "d3server.log")
     * @return True if initialized successfully, false otherwise
     */
    bool init(const std::string& logFilePath = "d3server.log");

    /**
     * @brief Log a message with the specified log level
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Set minimum log level to display
     * @param level Minimum log level
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Set whether to log to console
     * @param logToConsole True to log to console, false otherwise
     */
    void setLogToConsole(bool logToConsole);

    /**
     * @brief Register a log callback function
     * @param callback Function to call for each log message
     */
    void registerCallback(std::function<void(LogLevel, const std::string&)> callback);

    /**
     * @brief Close the logger
     */
    void close();

    /**
     * @brief Get log level as string
     * @param level Log level
     * @return String representation of log level
     */
    static std::string logLevelToString(LogLevel level);

private:
    Logger(); // Private constructor for singleton
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex m_mutex;
    std::ofstream m_logFile;
    LogLevel m_minLevel;
    bool m_logToConsole;
    std::vector<std::function<void(LogLevel, const std::string&)>> m_callbacks;
    bool m_initialized;
};

// Convenience macros for logging
#define LOG_DEBUG(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::DEBUG, message)

#define LOG_INFO(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::INFO, message)

#define LOG_WARNING(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::WARNING, message)

#define LOG_ERROR(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::ERROR, message)

#define LOG_CRITICAL(message) \
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::CRITICAL, message)

} // namespace core
} // namespace d3server
```

Now let's implement the logger class:

```cpp
#include "core/logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace d3server {
namespace core {

Logger::Logger() : m_minLevel(LogLevel::INFO), m_logToConsole(true), m_initialized(false) {}

Logger::~Logger() {
    close();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::init(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        // Already initialized
        return true;
    }
    
    m_logFile.open(logFilePath, std::ios::out | std::ios::app);
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        return false;
    }
    
    m_initialized = true;
    
    // Log initialization message
    log(LogLevel::INFO, "Logger initialized. Log file: " + logFilePath);
    
    return true;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream timeString;
    timeString << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S")
               << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    
    // Format the log message
    std::stringstream formattedMessage;
    formattedMessage << timeString.str() << " [" << logLevelToString(level) << "] " << message;
    
    // Write to file if initialized
    if (m_initialized && m_logFile.is_open()) {
        m_logFile << formattedMessage.str() << std::endl;
        m_logFile.flush();
    }
    
    // Write to console if enabled
    if (m_logToConsole) {
        // Select console output based on level
        if (level >= LogLevel::ERROR) {
            std::cerr << formattedMessage.str() << std::endl;
        } else {
            std::cout << formattedMessage.str() << std::endl;
        }
    }
    
    // Call registered callbacks
    for (const auto& callback : m_callbacks) {
        callback(level, formattedMessage.str());
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
    
    log(LogLevel::INFO, "Log level set to " + logLevelToString(level));
}

void Logger::setLogToConsole(bool logToConsole) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logToConsole = logToConsole;
    
    log(LogLevel::INFO, std::string("Console logging ") + 
        (logToConsole ? "enabled" : "disabled"));
}

void Logger::registerCallback(std::function<void(LogLevel, const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks.push_back(callback);
    
    log(LogLevel::INFO, "Log callback registered");
}

void Logger::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized && m_logFile.is_open()) {
        log(LogLevel::INFO, "Logger shutting down");
        m_logFile.close();
    }
    
    m_initialized = false;
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

} // namespace core
} // namespace d3server
```

---

**User**

继续完成代码，注意替代它的http客户端你，使用boost.asio 即可 

---

**Cursor**

I'll continue building the C++ implementation, focusing on the server and the REST API components using Boost.Asio. Let's start with implementing the core server and configuration classes.

First, let's create the core server header file:

```cpp
#pragma once

#include <memory>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace d3server {

// Forward declarations
namespace database {
class DatabaseManager;
}

namespace battle_net {
class BattleNetServer;
}

namespace game_server {
class GameServer;
}

namespace rest_api {
class RestServer;
}

namespace core {

class Config;

/**
 * @brief Main server class that coordinates all server components
 */
class Server {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     * @param battleNetServer Battle.net server instance
     * @param gameServer Game server instance
     * @param restServer REST API server instance
     */
    Server(
        std::shared_ptr<Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager,
        std::shared_ptr<battle_net::BattleNetServer> battleNetServer,
        std::shared_ptr<game_server::GameServer> gameServer,
        std::shared_ptr<rest_api::RestServer> restServer
    );

    /**
     * @brief Destructor
     */
    ~Server();

    /**
     * @brief Initialize the server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();

    /**
     * @brief Run the server (blocking call)
     */
    void run();

    /**
     * @brief Shutdown the server
     */
    void shutdown();

    /**
     * @brief Check if the server is running
     * @return True if the server is running, false otherwise
     */
    bool isRunning() const;

private:
    std::shared_ptr<Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<battle_net::BattleNetServer> m_battleNetServer;
    std::shared_ptr<game_server::GameServer> m_gameServer;
    std::shared_ptr<rest_api::RestServer> m_restServer;

    std::atomic<bool> m_running;
    std::vector<std::thread> m_serverThreads;

    /**
     * @brief Main server loop
     */
    void mainLoop();
};

} // namespace core
} // namespace d3server
```

Let's implement the server class:

```cpp
#include "core/server.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"
#include "battle_net/battle_net_server.h"
#include "game_server/game_server.h"
#include "rest_api/rest_server.h"

#include <chrono>
#include <thread>

namespace d3server {
namespace core {

Server::Server(
    std::shared_ptr<Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager,
    std::shared_ptr<battle_net::BattleNetServer> battleNetServer,
    std::shared_ptr<game_server::GameServer> gameServer,
    std::shared_ptr<rest_api::RestServer> restServer
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_battleNetServer(battleNetServer),
    m_gameServer(gameServer),
    m_restServer(restServer),
    m_running(false)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Server instance created");
    DEBUG_FUNCTION_EXIT();
}

Server::~Server() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("Server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool Server::init() {
    DEBUG_FUNCTION_ENTER();
    
    LOG_INFO("Initializing server...");
    
    // Initialize all components
    try {
        // Initialize Battle.net server
        DEBUG_CONTEXT("Initializing Battle.net server");
        if (!m_battleNetServer->init()) {
            LOG_ERROR("Failed to initialize Battle.net server");
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Initialize game server
        DEBUG_CONTEXT("Initializing game server");
        if (!m_gameServer->init()) {
            LOG_ERROR("Failed to initialize game server");
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Initialize REST API server
        DEBUG_CONTEXT("Initializing REST API server");
        if (!m_restServer->init()) {
            LOG_ERROR("Failed to initialize REST API server");
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        LOG_INFO("Server initialized successfully");
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Exception during server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void Server::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("Server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    LOG_INFO("Starting server components...");
    
    try {
        // Start Battle.net server in a separate thread
        m_serverThreads.emplace_back([this]() {
            try {
                DEBUG_CONTEXT("Starting Battle.net server thread");
                m_battleNetServer->run();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception in Battle.net server thread: " + std::string(e.what()));
            }
        });
        
        // Start game server in a separate thread
        m_serverThreads.emplace_back([this]() {
            try {
                DEBUG_CONTEXT("Starting game server thread");
                m_gameServer->run();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception in game server thread: " + std::string(e.what()));
            }
        });
        
        // Start REST API server in a separate thread
        m_serverThreads.emplace_back([this]() {
            try {
                DEBUG_CONTEXT("Starting REST API server thread");
                m_restServer->run();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception in REST API server thread: " + std::string(e.what()));
            }
        });
        
        LOG_INFO("All server components started");
        
        // Run main server loop
        mainLoop();
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Exception during server startup: " + std::string(e.what()));
        shutdown();
    }
    
    DEBUG_FUNCTION_EXIT();
}

void Server::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("Server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down server...");
    m_running = false;
    
    // Shutdown server components
    try {
        // Shutdown REST API server
        DEBUG_CONTEXT("Shutting down REST API server");
        m_restServer->shutdown();
        
        // Shutdown game server
        DEBUG_CONTEXT("Shutting down game server");
        m_gameServer->shutdown();
        
        // Shutdown Battle.net server
        DEBUG_CONTEXT("Shutting down Battle.net server");
        m_battleNetServer->shutdown();
        
        // Wait for all threads to finish
        for (auto& thread : m_serverThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        m_serverThreads.clear();
        
        LOG_INFO("Server shutdown complete");
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during server shutdown: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

bool Server::isRunning() const {
    return m_running;
}

void Server::mainLoop() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Entering main server loop");
    
    while (m_running) {
        // Perform periodic server tasks here
        
        // Sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    LOG_INFO("Exiting main server loop");
    DEBUG_FUNCTION_EXIT();
}

} // namespace core
} // namespace d3server
```

Now let's create the configuration class:

```cpp
#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace d3server {
namespace core {

/**
 * @brief Database configuration structure
 */
struct DatabaseConfig {
    std::string type;         // Database type (sqlite)
    std::string filePath;     // SQLite database file path
    std::string accountsDb;   // Account database name
    std::string worldsDb;     // Worlds database name
};

/**
 * @brief Network configuration structure
 */
struct NetworkConfig {
    std::string bindIp;       // IP address to bind to
    std::string publicIp;     // Public IP address
    int battleNetPort;        // Battle.net server port
    int gameServerPort;       // Game server port
    int restApiPort;          // REST API server port
    bool enableSsl;           // Whether to enable SSL
    std::string sslCertPath;  // Path to SSL certificate
    std::string sslKeyPath;   // Path to SSL private key
};

/**
 * @brief Server configuration structure
 */
struct ServerConfig {
    std::string serverName;         // Server name
    int maxPlayers;                 // Maximum number of players
    int maxAccountsPerIp;           // Maximum number of accounts per IP
    int maxCharactersPerAccount;    // Maximum number of characters per account
    std::string defaultLocale;      // Default locale
    std::string motd;               // Message of the day
    bool enableDebug;               // Whether debug mode is enabled
    std::string logLevel;           // Log level
    std::string logPath;            // Log file path
};

/**
 * @brief Configuration class for the server
 */
class Config {
public:
    /**
     * @brief Constructor
     */
    Config();

    /**
     * @brief Destructor
     */
    ~Config();

    /**
     * @brief Load configuration from a file
     * @param filePath Path to the configuration file
     * @return True if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filePath);

    /**
     * @brief Save configuration to a file
     * @param filePath Path to the configuration file
     * @return True if saved successfully, false otherwise
     */
    bool saveToFile(const std::string& filePath) const;

    /**
     * @brief Get the database configuration
     * @return Database configuration
     */
    const DatabaseConfig& getDatabaseConfig() const;

    /**
     * @brief Get the network configuration
     * @return Network configuration
     */
    const NetworkConfig& getNetworkConfig() const;

    /**
     * @brief Get the server configuration
     * @return Server configuration
     */
    const ServerConfig& getServerConfig() const;

    /**
     * @brief Set a configuration value
     * @param section Configuration section
     * @param key Configuration key
     * @param value Configuration value
     */
    void setValue(const std::string& section, const std::string& key, const std::string& value);

    /**
     * @brief Get a configuration value
     * @param section Configuration section
     * @param key Configuration key
     * @param defaultValue Default value to return if not found
     * @return Configuration value or default value if not found
     */
    std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;

private:
    /**
     * @brief Parse configuration values into structured config objects
     */
    void parseConfig();

    /**
     * @brief Set default configuration values
     */
    void setDefaults();

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_config;
    DatabaseConfig m_databaseConfig;
    NetworkConfig m_networkConfig;
    ServerConfig m_serverConfig;
};

} // namespace core
} // namespace d3server
```

Now let's implement the configuration class:

```cpp
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

#include <fstream>
#include <sstream>

namespace d3server {
namespace core {

Config::Config() {
    DEBUG_FUNCTION_ENTER();
    setDefaults();
    DEBUG_FUNCTION_EXIT();
}

Config::~Config() {
    DEBUG_FUNCTION_ENTER();
    DEBUG_FUNCTION_EXIT();
}

bool Config::loadFromFile(const std::string& filePath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    // Check if file exists
    if (!utils::FileUtils::fileExists(filePath)) {
        LOG_WARNING("Configuration file not found: " + filePath + ", using defaults");
        DEBUG_FUNCTION_EXIT();
        return false;
    }
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open configuration file: " + filePath);
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        std::string line;
        std::string currentSection;
        
        // Read file line by line
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            // Trim whitespace
            line = utils::StringUtils::trim(line);
            
            // Check for section
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            
            // Parse key=value
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = utils::StringUtils::trim(line.substr(0, pos));
                std::string value = utils::StringUtils::trim(line.substr(pos + 1));
                
                // Store in config map
                m_config[currentSection][key] = value;
            }
        }
        
        file.close();
        
        // Parse configuration values into structured configs
        parseConfig();
        
        LOG_INFO("Configuration loaded from: " + filePath);
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception loading configuration: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

bool Config::saveToFile(const std::string& filePath) const {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open configuration file for writing: " + filePath);
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Write configuration file header
        file << "# D3Server Configuration File" << std::endl;
        file << "# Generated on " << utils::StringUtils::getCurrentTimeString() << std::endl;
        file << std::endl;
        
        // Write sections and key-value pairs
        for (const auto& section : m_config) {
            // Skip empty sections
            if (section.second.empty()) {
                continue;
            }
            
            // Write section header
            file << "[" << section.first << "]" << std::endl;
            
            // Write key-value pairs
            for (const auto& keyValue : section.second) {
                file << keyValue.first << " = " << keyValue.second << std::endl;
            }
            
            file << std::endl;
        }
        
        file.close();
        
        LOG_INFO("Configuration saved to: " + filePath);
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception saving configuration: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

const DatabaseConfig& Config::getDatabaseConfig() const {
    return m_databaseConfig;
}

const NetworkConfig& Config::getNetworkConfig() const {
    return m_networkConfig;
}

const ServerConfig& Config::getServerConfig() const {
    return m_serverConfig;
}

void Config::setValue(const std::string& section, const std::string& key, const std::string& value) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(section);
    DEBUG_VARIABLE(key);
    DEBUG_VARIABLE(value);
    
    m_config[section][key] = value;
    parseConfig();
    
    DEBUG_FUNCTION_EXIT();
}

std::string Config::getValue(const std::string& section, const std::string& key, const std::string& defaultValue) const {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(section);
    DEBUG_VARIABLE(key);
    DEBUG_VARIABLE(defaultValue);
    
    // Check if section exists
    auto sectionIt = m_config.find(section);
    if (sectionIt != m_config.end()) {
        // Check if key exists
        auto keyIt = sectionIt->second.find(key);
        if (keyIt != sectionIt->second.end()) {
            DEBUG_FUNCTION_EXIT();
            return keyIt->second;
        }
    }
    
    DEBUG_FUNCTION_EXIT();
    return defaultValue;
}

void Config::parseConfig() {
    DEBUG_FUNCTION_ENTER();
    
    // Parse database configuration
    m_databaseConfig.type = getValue("Database", "Type", "sqlite");
    m_databaseConfig.filePath = getValue("Database", "FilePath", "d3server.db");
    m_databaseConfig.accountsDb = getValue("Database", "AccountsDB", "accounts");
    m_databaseConfig.worldsDb = getValue("Database", "WorldsDB", "worlds");
    
    // Parse network configuration
    m_networkConfig.bindIp = getValue("Network", "BindIP", "0.0.0.0");
    m_networkConfig.publicIp = getValue("Network", "PublicIP", "127.0.0.1");
    m_networkConfig.battleNetPort = std::stoi(getValue("Network", "BattleNetPort", "1119"));
    m_networkConfig.gameServerPort = std::stoi(getValue("Network", "GameServerPort", "1120"));
    m_networkConfig.restApiPort = std::stoi(getValue("Network", "RestApiPort", "8080"));
    m_networkConfig.enableSsl = (getValue("Network", "EnableSSL", "true") == "true");
    m_networkConfig.sslCertPath = getValue("Network", "SSLCertPath", "certs/server.crt");
    m_networkConfig.sslKeyPath = getValue("Network", "SSLKeyPath", "certs/server.key");
    
    // Parse server configuration
    m_serverConfig.serverName = getValue("Server", "ServerName", "D3Server");
    m_serverConfig.maxPlayers = std::stoi(getValue("Server", "MaxPlayers", "1000"));
    m_serverConfig.maxAccountsPerIp = std::stoi(getValue("Server", "MaxAccountsPerIP", "10"));
    m_serverConfig.maxCharactersPerAccount = std::stoi(getValue("Server", "MaxCharactersPerAccount", "10"));
    m_serverConfig.defaultLocale = getValue("Server", "DefaultLocale", "enUS");
    m_serverConfig.motd = getValue("Server", "MOTD", "Welcome to D3Server!");
    m_serverConfig.enableDebug = (getValue("Server", "EnableDebug", "false") == "true");
    m_serverConfig.logLevel = getValue("Server", "LogLevel", "INFO");
    m_serverConfig.logPath = getValue("Server", "LogPath", "logs/d3server.log");
    
    DEBUG_FUNCTION_EXIT();
}

void Config::setDefaults() {
    DEBUG_FUNCTION_ENTER();
    
    // Database defaults
    setValue("Database", "Type", "sqlite");
    setValue("Database", "FilePath", "d3server.db");
    setValue("Database", "AccountsDB", "accounts");
    setValue("Database", "WorldsDB", "worlds");
    
    // Network defaults
    setValue("Network", "BindIP", "0.0.0.0");
    setValue("Network", "PublicIP", "127.0.0.1");
    setValue("Network", "BattleNetPort", "1119");
    setValue("Network", "GameServerPort", "1120");
    setValue("Network", "RestApiPort", "8080");
    setValue("Network", "EnableSSL", "true");
    setValue("Network", "SSLCertPath", "certs/server.crt");
    setValue("Network", "SSLKeyPath", "certs/server.key");
    
    // Server defaults
    setValue("Server", "ServerName", "D3Server");
    setValue("Server", "MaxPlayers", "1000");
    setValue("Server", "MaxAccountsPerIP", "10");
    setValue("Server", "MaxCharactersPerAccount", "10");
    setValue("Server", "DefaultLocale", "enUS");
    setValue("Server", "MOTD", "Welcome to D3Server!");
    setValue("Server", "EnableDebug", "false");
    setValue("Server", "LogLevel", "INFO");
    setValue("Server", "LogPath", "logs/d3server.log");
    
    // Parse defaults into structured configs
    parseConfig();
    
    DEBUG_FUNCTION_EXIT();
}

} // namespace core
} // namespace d3server
```

Now let's implement the string utils and file utils classes that the config class depends on:

```cpp
#pragma once

#include <string>
#include <vector>

namespace d3server {
namespace utils {

/**
 * @brief Utility class for string operations
 */
class StringUtils {
public:
    /**
     * @brief Trim whitespace from the beginning and end of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);

    /**
     * @brief Trim whitespace from the beginning of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trimLeft(const std::string& str);

    /**
     * @brief Trim whitespace from the end of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trimRight(const std::string& str);

    /**
     * @brief Split a string into tokens using a delimiter
     * @param str String to split
     * @param delimiter Delimiter to split by
     * @return Vector of tokens
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Join a vector of strings using a delimiter
     * @param strings Vector of strings to join
     * @param delimiter Delimiter to join with
     * @return Joined string
     */
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

    /**
     * @brief Convert a string to lowercase
     * @param str String to convert
     * @return Lowercase string
     */
    static std::string toLower(const std::string& str);

    /**
     * @brief Convert a string to uppercase
     * @param str String to convert
     * @return Uppercase string
     */
    static std::string toUpper(const std::string& str);

    /**
     * @brief Check if a string starts with a prefix
     * @param str String to check
     * @param prefix Prefix to check for
     * @return True if the string starts with the prefix, false otherwise
     */
    static bool startsWith(const std::string& str, const std::string& prefix);

    /**
     * @brief Check if a string ends with a suffix
     * @param str String to check
     * @param suffix Suffix to check for
     * @return True if the string ends with the suffix, false otherwise
     */
    static bool endsWith(const std::string& str, const std::string& suffix);

    /**
     * @brief Replace all occurrences of a substring in a string
     * @param str String to modify
     * @param from Substring to replace
     * @param to Replacement substring
     * @return Modified string
     */
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);

    /**
     * @brief Get current time as a formatted string
     * @param format Format string (default: "%Y-%m-%d %H:%M:%S")
     * @return Formatted time string
     */
    static std::string getCurrentTimeString(const std::string& format = "%Y-%m-%d %H:%M:%S");
};

} // namespace utils
} // namespace d3server
```

```cpp
#include "utils/string_utils.h"
#include "utils/debug.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace d3server {
namespace utils {

std::string StringUtils::trim(const std::string& str) {
    return trimRight(trimLeft(str));
}

std::string StringUtils::trimLeft(const std::string& str) {
    auto it = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace(static_cast<unsigned char>(ch));
    });
    return std::string(it, str.end());
}

std::string StringUtils::trimRight(const std::string& str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return !std::isspace(static_cast<unsigned char>(ch));
    }).base();
    return std::string(str.begin(), it);
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    DEBUG_FUNCTION_ENTER();
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    DEBUG_FUNCTION_EXIT();
    return tokens;
}

std::string StringUtils::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    DEBUG_FUNCTION_ENTER();
    std::ostringstream result;
    
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) {
            result << delimiter;
        }
        result << strings[i];
    }
    
    DEBUG_FUNCTION_EXIT();
    return result.str();
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string StringUtils::replace(const std::string& str, const std::string& from, const std::string& to) {
    DEBUG_FUNCTION_ENTER();
    std::string result = str;
    size_t startPos = 0;
    
    while ((startPos = result.find(from, startPos)) != std::string::npos) {
        result.replace(startPos, from.length(), to);
        startPos += to.length();
    }
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string StringUtils::getCurrentTimeString(const std::string& format) {
    DEBUG_FUNCTION_ENTER();
    
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), format.c_str());
    
    DEBUG_FUNCTION_EXIT();
    return ss.str();
}

} // namespace utils
} // namespace d3server
```

```cpp
#pragma once

#include <string>
#include <vector>

namespace d3server {
namespace utils {

/**
 * @brief Utility class for file operations
 */
class FileUtils {
public:
    /**
     * @brief Check if a file exists
     * @param filePath Path to the file
     * @return True if the file exists, false otherwise
     */
    static bool fileExists(const std::string& filePath);

    /**
     * @brief Check if a directory exists
     * @param dirPath Path to the directory
     * @return True if the directory exists, false otherwise
     */
    static bool directoryExists(const std::string& dirPath);

    /**
     * @brief Create a directory
     * @param dirPath Path to the directory
     * @param recursive Whether to create parent directories if they don't exist
     * @return True if the directory was created or already exists, false otherwise
     */
    static bool createDirectory(const std::string& dirPath, bool recursive = true);

    /**
     * @brief Read a file into a string
     * @param filePath Path to the file
     * @param content Output variable for the file content
     * @return True if the file was read successfully, false otherwise
     */
    static bool readFile(const std::string& filePath, std::string& content);

    /**
     * @brief Read a file into a vector of lines
     * @param filePath Path to the file
     * @param lines Output variable for the file lines
     * @return True if the file was read successfully, false otherwise
     */
    static bool readFileLines(const std::string& filePath, std::vector<std::string>& lines);

    /**
     * @brief Write a string to a file
     * @param filePath Path to the file
     * @param content Content to write
     * @param append Whether to append to the file (true) or overwrite it (false)
     * @return True if the file was written successfully, false otherwise
     */
    static bool writeFile(const std::string& filePath, const std::string& content, bool append = false);

    /**
     * @brief Write a vector of lines to a file
     * @param filePath Path to the file
     * @param lines Lines to write
     * @param append Whether to append to the file (true) or overwrite it (false)
     * @return True if the file was written successfully, false otherwise
     */
    static bool writeFileLines(const std::string& filePath, const std::vector<std::string>& lines, bool append = false);

    /**
     * @brief Delete a file
     * @param filePath Path to the file
     * @return True if the file was deleted successfully, false otherwise
     */
    static bool deleteFile(const std::string& filePath);

    /**
     * @brief Get the basename of a path
     * @param path Path to get the basename from
     * @return Basename of the path
     */
    static std::string getBasename(const std::string& path);

    /**
     * @brief Get the directory name of a path
     * @param path Path to get the directory name from
     * @return Directory name of the path
     */
    static std::string getDirname(const std::string& path);

    /**
     * @brief Get the extension of a file
     * @param path Path to get the extension from
     * @return Extension of the file
     */
    static std::string getExtension(const std::string& path);

    /**
     * @brief Normalize a path
     * @param path Path to normalize
     * @return Normalized path
     */
    static std::string normalizePath(const std::string& path);

    /**
     * @brief Join path components
     * @param components Path components to join
     * @return Joined path
     */
    static std::string joinPath(const std::vector<std::string>& components);

    /**
     * @brief Join two path components
     * @param path1 First path component
     * @param path2 Second path component
     * @return Joined path
     */
    static std::string joinPath(const std::string& path1, const std::string& path2);
};

} // namespace utils
} // namespace d3server
```

```cpp
#include "utils/file_utils.h"
#include "utils/debug.h"
#include "utils/string_utils.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace d3server {
namespace utils {

bool FileUtils::fileExists(const std::string& filePath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception checking if file exists: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::directoryExists(const std::string& dirPath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(dirPath);
    
    try {
        return fs::exists(dirPath) && fs::is_directory(dirPath);
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception checking if directory exists: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::createDirectory(const std::string& dirPath, bool recursive) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(dirPath);
    DEBUG_VARIABLE(recursive);
    
    try {
        if (directoryExists(dirPath)) {
            DEBUG_LOG("Directory already exists");
            return true;
        }
        
        if (recursive) {
            return fs::create_directories(dirPath);
        }
        else {
            return fs::create_directory(dirPath);
        }
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception creating directory: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::readFile(const std::string& filePath, std::string& content) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for reading");
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception reading file: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::readFileLines(const std::string& filePath, std::vector<std::string>& lines) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for reading lines");
            return false;
        }
        
        lines.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception reading file lines: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::writeFile(const std::string& filePath, const std::string& content, bool append) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    DEBUG_VARIABLE(append);
    
    try {
        // Create directory if it doesn't exist
        std::string dirName = getDirname(filePath);
        if (!dirName.empty() && !directoryExists(dirName)) {
            if (!createDirectory(dirName)) {
                DEBUG_LOG("Failed to create directory for file");
                return false;
            }
        }
        
        std::ios_base::openmode mode = std::ios::out | std::ios::binary;
        if (append) {
            mode |= std::ios::app;
        }
        
        std::ofstream file(filePath, mode);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for writing");
            return false;
        }
        
        file << content;
        file.close();
        
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception writing file: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::writeFileLines(const std::string& filePath, const std::vector<std::string>& lines, bool append) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    DEBUG_VARIABLE(append);
    
    try {
        // Create directory if it doesn't exist
        std::string dirName = getDirname(filePath);
        if (!dirName.empty() && !directoryExists(dirName)) {
            if (!createDirectory(dirName)) {
                DEBUG_LOG("Failed to create directory for file");
                return false;
            }
        }
        
        std::ios_base::openmode mode = std::ios::out;
        if (append) {
            mode |= std::ios::app;
        }
        
        std::ofstream file(filePath, mode);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for writing lines");
            return false;
        }
        
        for (const auto& line : lines) {
            file << line << std::endl;
        }
        
        file.close();
        
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception writing file lines: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::deleteFile(const std::string& filePath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        return fs::remove(filePath);
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception deleting file: " + std::string(e.what()));
        return false;
    }
}

std::string FileUtils::getBasename(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).filename().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception getting basename: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::getDirname(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).parent_path().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception getting dirname: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::getExtension(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).extension().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception getting extension: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::normalizePath(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).lexically_normal().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception normalizing path: " + std::string(e.what()));
        return path;
    }
}

std::string FileUtils::joinPath(const std::vector<std::string>& components) {
    DEBUG_FUNCTION_ENTER();
    
    if (components.empty()) {
        return "";
    }
    
    try {
        fs::path result(components[0]);
        
        for (size_t i = 1; i < components.size(); ++i) {
            result /= components[i];
        }
        
        return result.string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception joining paths: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::joinPath(const std::string& path1, const std::string& path2) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path1);
    DEBUG_VARIABLE(path2);
    
    try {
        return (fs::path(path1) / fs::path(path2)).string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception joining paths: " + std::string(e.what()));
        return "";
    }
}

} // namespace utils
} // namespace d3server
```

Now, let's implement the cryptography utilities which will be needed for authentication:

```cpp
#pragma once

#include <string>
#include <vector>

namespace d3server {
namespace utils {

/**
 * @brief Utility class for cryptography operations
 */
class CryptoUtils {
public:
    /**
     * @brief Compute SHA-1 hash of a string
     * @param data Input data
     * @return SHA-1 hash as hex string
     */
    static std::string sha1(const std::string& data);

    /**
     * @brief Compute SHA-256 hash of a string
     * @param data Input data
     * @return SHA-256 hash as hex string
     */
    static std::string sha256(const std::string& data);

    /**
     * @brief Compute MD5 hash of a string
     * @param data Input data
     * @return MD5 hash as hex string
     */
    static std::string md5(const std::string& data);

    /**
     * @brief Generate a random string
     * @param length Length of the string to generate
     * @param includeSpecialChars Whether to include special characters
     * @return Random string
     */
    static std::string generateRandomString(size_t length, bool includeSpecialChars = false);

    /**
     * @brief Generate a secure random bytes
     * @param length Length of the bytes to generate
     * @return Random bytes as vector
     */
    static std::vector<uint8_t> generateRandomBytes(size_t length);

    /**
     * @brief Base64 encode a string
     * @param data Input data
     * @return Base64 encoded string
     */
    static std::string base64Encode(const std::string& data);

    /**
     * @brief Base64 decode a string
     * @param data Base64 encoded string
     * @return Decoded string
     */
    static std::string base64Decode(const std::string& data);

    /**
     * @brief Encrypt a string using AES-256
     * @param data String to encrypt
     * @param key Encryption key
     * @param iv Initialization vector
     * @return Encrypted string
     */
    static std::string aesEncrypt(const std::string& data, const std::string& key, const std::string& iv);

    /**
     * @brief Decrypt a string using AES-256
     * @param data String to decrypt
     * @param key Encryption key
     * @param iv Initialization vector
     * @return Decrypted string
     */
    static std::string aesDecrypt(const std::string& data, const std::string& key, const std::string& iv);

    /**
     * @brief Hex encode a byte array
     * @param data Byte array to encode
     * @param length Length of the byte array
     * @return Hex encoded string
     */
    static std::string hexEncode(const uint8_t* data, size_t length);

    /**
     * @brief Hex decode a string
     * @param hexString Hex encoded string
     * @return Decoded byte vector
     */
    static std::vector<uint8_t> hexDecode(const std::string& hexString);

    /**
     * @brief Hash a password with a salt
     * @param password Password to hash
     * @param salt Salt to use (if empty, a random salt will be generated)
     * @return Hashed password with salt
     */
    static std::string hashPassword(const std::string& password, const std::string& salt = "");

    /**
     * @brief Verify a password against a hash
     * @param password Password to verify
     * @param passwordHash Password hash to verify against
     * @return True if the password matches the hash, false otherwise
     */
    static bool verifyPassword(const std::string& password, const std::string& passwordHash);
};

} // namespace utils
} // namespace d3server
```

```cpp
#include "utils/crypto_utils.h"
#include "utils/debug.h"

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace d3server {
namespace utils {

std::string CryptoUtils::sha1(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    uint8_t hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    
    std::string result = hexEncode(hash, SHA_DIGEST_LENGTH);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::sha256(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    
    std::string result = hexEncode(hash, SHA256_DIGEST_LENGTH);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::md5(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    uint8_t hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    
    std::string result = hexEncode(hash, MD5_DIGEST_LENGTH);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::generateRandomString(size_t length, bool includeSpecialChars) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(length);
    DEBUG_VARIABLE(includeSpecialChars);
    
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    static const char specialChars[] = "!@#$%^&*()-_=+[]{};:,.<>?";
    
    std::string charset = alphanum;
    if (includeSpecialChars) {
        charset += specialChars;
    }
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.size() - 1);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::vector<uint8_t> CryptoUtils::generateRandomBytes(size_t length) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(length);
    
    std::vector<uint8_t> buffer(length);
    
    if (RAND_bytes(buffer.data(), static_cast<int>(length)) != 1) {
        DEBUG_LOG("RAND_bytes failed, falling back to std::random_device");
        
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, 255);
        
        for (size_t i = 0; i < length; ++i) {
            buffer[i] = static_cast<uint8_t>(distribution(generator));
        }
    }
    
    DEBUG_FUNCTION_EXIT();
    return buffer;
}

std::string CryptoUtils::base64Encode(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, data.c_str(), static_cast<int>(data.length()));
    BIO_flush(b64);
    
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    
    std::string result(bptr->data, bptr->length - 1);  // -1 to remove the trailing newline
    
    BIO_free_all(b64);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::base64Decode(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(data.c_str(), static_cast<int>(data.length()));
    bmem = BIO_push(b64, bmem);
    
    std::vector<char> buffer(data.length());
    int decodedLength = BIO_read(bmem, buffer.data(), static_cast<int>(data.length()));
    
    BIO_free_all(bmem);
    
    std::string result(buffer.data(), decodedLength > 0 ? decodedLength : 0);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::aesEncrypt(const std::string& data, const std::string& key, const std::string& iv) {
    DEBUG_FUNCTION_ENTER();
    
    // Validate key and IV sizes
    if (key.length() != 32) {
        DEBUG_LOG("Invalid key size for AES-256");
        return "";
    }
    
    if (iv.length() != 16) {
        DEBUG_LOG("Invalid IV size for AES-256");
        return "";
    }
    
    // Initialize context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        DEBUG_LOG("Failed to create EVP_CIPHER_CTX");
        return "";
    }
    
    // Initialize encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                         reinterpret_cast<const uint8_t*>(key.c_str()),
                         reinterpret_cast<const uint8_t*>(iv.c_str())) != 1) {
        DEBUG_LOG("Failed to initialize encryption");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    // Prepare output buffer
    std::vector<uint8_t> ciphertext(data.length() + AES_BLOCK_SIZE);
    int ciphertextLength = 0;
    
    // Encrypt data
    if (EVP_EncryptUpdate(ctx,
                        ciphertext.data(), &ciphertextLength,
                        reinterpret_cast<const uint8_t*>(data.c_str()), static_cast<int>(data.length())) != 1) {
        DEBUG_LOG("Failed during encryption update");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    int finalLength = 0;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + ciphertextLength, &finalLength) != 1) {
        DEBUG_LOG("Failed during encryption finalization");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    ciphertextLength += finalLength;
    EVP_CIPHER_CTX_free(ctx);
    
    // Return base64-encoded ciphertext
    std::string result = base64Encode(std::string(reinterpret_cast<char*>(ciphertext.data()), ciphertextLength));
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::aesDecrypt(const std::string& data, const std::string& key, const std::string& iv) {
    DEBUG_FUNCTION_ENTER();
    
    // Validate key and IV sizes
    if (key.length() != 32) {
        DEBUG_LOG("Invalid key size for AES-256");
        return "";
    }
    
    if (iv.length() != 16) {
        DEBUG_LOG("Invalid IV size for AES-256");
        return "";
    }
    
    // Decode base64
    std::string ciphertext = base64Decode(data);
    
    // Initialize context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        DEBUG_LOG("Failed to create EVP_CIPHER_CTX");
        return "";
    }
    
    // Initialize decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                         reinterpret_cast<const uint8_t*>(key.c_str()),
                         reinterpret_cast<const uint8_t*>(iv.c_str())) != 1) {
        DEBUG_LOG("Failed to initialize decryption");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    // Prepare output buffer
    std::vector<uint8_t> plaintext(ciphertext.length());
    int plaintextLength = 0;
    
    // Decrypt data
    if (EVP_DecryptUpdate(ctx,
                        plaintext.data(), &plaintextLength,
                        reinterpret_cast<const uint8_t*>(ciphertext.c_str()), static_cast<int>(ciphertext.length())) != 1) {
        DEBUG_LOG("Failed during decryption update");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    int finalLength = 0;
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintextLength, &finalLength) != 1) {
        DEBUG_LOG("Failed during decryption finalization");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    plaintextLength += finalLength;
    EVP_CIPHER_CTX_free(ctx);
    
    std::string result(reinterpret_cast<char*>(plaintext.data()), plaintextLength);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::hexEncode(const uint8_t* data, size_t length) {
    DEBUG_FUNCTION_ENTER();
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    
    DEBUG_FUNCTION_EXIT();
    return ss.str();
}

std::vector<uint8_t> CryptoUtils::hexDecode(const std::string& hexString) {
    DEBUG_FUNCTION_ENTER();
    
    std::vector<uint8_t> result;
    for (size_t i = 0; i < hexString.length(); i += 2) {
        std::string byteString = hexString.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        result.push_back(byte);
    }
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::hashPassword(const std::string& password, const std::string& salt) {
    DEBUG_FUNCTION_ENTER();
    
    // Generate a random salt if none provided
    std::string useSalt = salt.empty() 
        ? base64Encode(std::string(reinterpret_cast<char*>(generateRandomBytes(16).data()), 16))
        : salt;
    
    // Combine password and salt
    std::string combined = password + "$" + useSalt;
    
    // Hash using SHA-256
    std::string hash = sha256(combined);
    
    // Format as salt$hash
    std::string result = useSalt + "$" + hash;
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

bool CryptoUtils::verifyPassword(const std::string& password, const std::string& passwordHash) {
    DEBUG_FUNCTION_ENTER();
    
    // Split hash into salt and hash
    size_t separatorPos = passwordHash.find('$');
    if (separatorPos == std::string::npos) {
        DEBUG_LOG("Invalid password hash format");
        return false;
    }
    
    std::string salt = passwordHash.substr(0, separatorPos);
    std::string expectedHash = passwordHash.substr(separatorPos + 1);
    
    // Combine password and salt
    std::string combined = password + "$" + salt;
    
    // Hash
    std::string computedHash = sha256(combined);
    
    // Compare
    bool result = (computedHash == expectedHash);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

} // namespace utils
} // namespace d3server
```

Now, let's start implementing the REST API module using Boost.Asio as requested. First, let's create the REST server header file:

```cpp
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace rest_api {

class AccountController;
class AdminController;

/**
 * @brief Represents an HTTP request
 */
struct HttpRequest {
    std::string method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

/**
 * @brief Represents an HTTP response
 */
struct HttpResponse {
    int statusCode;
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

/**
 * @brief Function type for API route handlers
 */
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief REST API server that handles HTTP requests
 */
class RestServer {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    RestServer(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );

    /**
     * @brief Destructor
     */
    ~RestServer();

    /**
     * @brief Initialize the REST server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();

    /**
     * @brief Run the REST server (blocking call)
     */
    void run();

    /**
     * @brief Shutdown the REST server
     */
    void shutdown();

private:
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(boost::asio::ip::tcp::socket socket, RestServer& server);
        void start();
        
    private:
        void readRequest();
        void processRequest(const HttpRequest& request);
        void writeResponse(const HttpResponse& response);
        
        boost::asio::ip::tcp::socket m_socket;
        RestServer& m_server;
        boost::asio::streambuf m_requestBuffer;
        std::string m_response;
    };
    
    /**
     * @brief Start accepting connections
     */
    void startAccept();
    
    /**
     * @brief Handle a new connection
     * @param socket Socket for the new connection
     */
    void handleAccept(boost::asio::ip::tcp::socket socket);
    
    /**
     * @brief Parse an HTTP request
     * @param data Request data
     * @return Parsed HTTP request
     */
    HttpRequest parseRequest(const std::string& data);
    
    /**
     * @brief Format an HTTP response
     * @param response HTTP response
     * @return Formatted HTTP response
     */
    std::string formatResponse(const HttpResponse& response);
    
    /**
     * @brief Register API routes
     */
    void registerRoutes();
    
    /**
     * @brief Create a JSON response
     * @param statusCode HTTP status code
     * @param body JSON body
     * @return HTTP response
     */
    HttpResponse createJsonResponse(int statusCode, const std::string& body);
    
    /**
     * @brief Handle unauthorized access
     * @return HTTP response for unauthorized access
     */
    HttpResponse handleUnauthorized();
    
    /**
     * @brief Check if a request is authorized
     * @param request HTTP request
     * @return True if authorized, false otherwise
     */
    bool isAuthorized(const HttpRequest& request);
    
    /**
     * @brief Extract API key from request
     * @param request HTTP request
     * @return API key or empty string if not found
     */
    std::string getApiKey(const HttpRequest& request);
    
    /**
     * @brief Validate an API key
     * @param apiKey API key to validate
     * @return True if valid, false otherwise
     */
    bool validateApiKey(const std::string& apiKey);
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<AccountController> m_accountController;
    std::shared_ptr<AdminController> m_adminController;
    
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    std::atomic<bool> m_running;
    std::thread m_ioThread;
    std::mutex m_routesMutex;
    
    // Routes map: method -> (uri -> handler)
    std::unordered_map<std::string, std::unordered_map<std::string, RouteHandler>> m_routes;
};

} // namespace rest_api
} // namespace d3server
```

Now let's implement the REST server:

---

**User**

继续完善各个模块，例如战网， 游戏服务端，rest等等模块都进行充分完善并详细说明你编码的过程和细节，输出对应的文档到 doc

---

**Cursor**

I'll continue implementing the REST API server module using Boost.Asio, followed by completing the rest of the modules. Let's start with the REST server implementation.

```cpp
#include "rest_api/rest_server.h"
#include "rest_api/account_controller.h"
#include "rest_api/admin_controller.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "database/database_manager.h"

#include <iostream>
#include <sstream>
#include <regex>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace d3server {
namespace rest_api {

// Session implementation
RestServer::Session::Session(boost::asio::ip::tcp::socket socket, RestServer& server)
    : m_socket(std::move(socket)), m_server(server) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::start() {
    DEBUG_FUNCTION_ENTER();
    readRequest();
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::readRequest() {
    DEBUG_FUNCTION_ENTER();
    
    auto self = shared_from_this();
    boost::asio::async_read_until(
        m_socket,
        m_requestBuffer,
        "\r\n\r\n",
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                DEBUG_LOG("Request header received, reading body");
                
                // Read the request data into a string
                std::istream request_stream(&m_requestBuffer);
                std::string request_data;
                std::string line;
                std::size_t content_length = 0;
                
                // Parse headers
                while (std::getline(request_stream, line) && line != "\r") {
                    request_data += line + "\n";
                    
                    // Look for Content-Length header
                    std::regex content_length_regex("Content-Length:\\s*(\\d+)", std::regex::icase);
                    std::smatch match;
                    if (std::regex_search(line, match, content_length_regex)) {
                        content_length = std::stoi(match[1]);
                    }
                }
                
                // If there's a body, read it
                if (content_length > 0) {
                    // Calculate how many bytes are already in the buffer
                    std::size_t body_bytes_available = m_requestBuffer.size();
                    
                    if (body_bytes_available < content_length) {
                        // Need to read more data
                        boost::asio::async_read(
                            m_socket,
                            m_requestBuffer,
                            boost::asio::transfer_exactly(content_length - body_bytes_available),
                            [this, self, request_data, content_length](boost::system::error_code ec, std::size_t) {
                                if (!ec) {
                                    std::string body(std::istreambuf_iterator<char>(&m_requestBuffer), {});
                                    processRequest(m_server.parseRequest(request_data + "\n" + body));
                                } else {
                                    DEBUG_LOG("Error reading request body: " + ec.message());
                                }
                            }
                        );
                    } else {
                        // All data already available
                        std::string body(std::istreambuf_iterator<char>(&m_requestBuffer), {});
                        processRequest(m_server.parseRequest(request_data + "\n" + body));
                    }
                } else {
                    // No body, process the request
                    processRequest(m_server.parseRequest(request_data));
                }
            } else {
                DEBUG_LOG("Error reading request: " + ec.message());
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::processRequest(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    // Get handler for this route
    std::lock_guard<std::mutex> lock(m_server.m_routesMutex);
    
    // Find method handler
    auto methodIt = m_server.m_routes.find(request.method);
    if (methodIt != m_server.m_routes.end()) {
        // Find URI handler
        auto uriIt = methodIt->second.find(request.uri);
        if (uriIt != methodIt->second.end()) {
            // Found a direct match
            HttpResponse response = uriIt->second(request);
            writeResponse(response);
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Try to find a pattern match
        for (const auto& routePair : methodIt->second) {
            const std::string& pattern = routePair.first;
            
            // Skip non-pattern routes
            if (pattern.find('{') == std::string::npos) {
                continue;
            }
            
            // Convert pattern to regex
            std::string regexPattern = pattern;
            std::regex paramRegex("\\{([^\\}]+)\\}");
            regexPattern = std::regex_replace(regexPattern, paramRegex, "([^/]+)");
            regexPattern = "^" + regexPattern + "$";
            
            std::regex pathRegex(regexPattern);
            std::smatch matches;
            
            if (std::regex_match(request.uri, matches, pathRegex)) {
                // Found a pattern match
                HttpResponse response = routePair.second(request);
                writeResponse(response);
                DEBUG_FUNCTION_EXIT();
                return;
            }
        }
    }
    
    // No handler found, return 404
    HttpResponse response;
    response.statusCode = 404;
    response.statusMessage = "Not Found";
    response.headers["Content-Type"] = "application/json";
    response.body = "{\"error\":\"Not Found\",\"message\":\"The requested resource was not found\"}";
    
    writeResponse(response);
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::writeResponse(const HttpResponse& response) {
    DEBUG_FUNCTION_ENTER();
    
    auto self = shared_from_this();
    m_response = m_server.formatResponse(response);
    
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_response),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                DEBUG_LOG("Error writing response: " + ec.message());
            }
            
            // Close the socket after sending the response
            boost::system::error_code ignored_ec;
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

// RestServer implementation
RestServer::RestServer(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_ioContext(),
    m_acceptor(m_ioContext),
    m_running(false)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("REST API server instance created");
    DEBUG_FUNCTION_EXIT();
}

RestServer::~RestServer() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("REST API server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool RestServer::init() {
    DEBUG_FUNCTION_ENTER();
    
    try {
        LOG_INFO("Initializing REST API server...");
        
        // Create controllers
        m_accountController = std::make_shared<AccountController>(m_config, m_dbManager);
        m_adminController = std::make_shared<AdminController>(m_config, m_dbManager);
        
        // Register API routes
        registerRoutes();
        
        // Initialize acceptor
        auto& networkConfig = m_config->getNetworkConfig();
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(networkConfig.bindIp),
            networkConfig.restApiPort
        );
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
        
        LOG_INFO("REST API server initialized on " + networkConfig.bindIp + ":" + std::to_string(networkConfig.restApiPort));
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during REST API server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void RestServer::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("REST API server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    
    LOG_INFO("Starting REST API server...");
    
    // Start accepting connections
    startAccept();
    
    // Run the IO context
    m_ioThread = std::thread([this]() {
        DEBUG_CONTEXT("REST API server IO thread started");
        
        try {
            m_ioContext.run();
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception in REST API server IO thread: " + std::string(e.what()));
        }
        
        DEBUG_LOG("REST API server IO thread exited");
    });
    
    LOG_INFO("REST API server started");
    DEBUG_FUNCTION_EXIT();
}

void RestServer::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("REST API server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down REST API server...");
    m_running = false;
    
    // Stop the IO context
    m_ioContext.stop();
    
    // Wait for the IO thread to finish
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
    
    LOG_INFO("REST API server shutdown complete");
    DEBUG_FUNCTION_EXIT();
}

void RestServer::startAccept() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        DEBUG_LOG("REST API server is not running, not accepting connections");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                // Create a new session
                std::make_shared<Session>(std::move(socket), *this)->start();
            } else {
                DEBUG_LOG("Error accepting connection: " + ec.message());
            }
            
            // Accept the next connection
            startAccept();
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

HttpRequest RestServer::parseRequest(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    HttpRequest request;
    std::istringstream stream(data);
    std::string line;
    
    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream request_line(line);
        request_line >> request.method >> request.uri >> request.version;
    }
    
    // Parse headers
    bool inHeaders = true;
    while (inHeaders && std::getline(stream, line)) {
        // Check for end of headers
        if (line == "\r" || line.empty()) {
            inHeaders = false;
            continue;
        }
        
        // Parse header
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string name = utils::StringUtils::trim(line.substr(0, colon));
            std::string value = utils::StringUtils::trim(line.substr(colon + 1));
            request.headers[name] = value;
        }
    }
    
    // Parse body
    if (!inHeaders) {
        std::stringstream body_stream;
        body_stream << stream.rdbuf();
        request.body = body_stream.str();
    }
    
    DEBUG_FUNCTION_EXIT();
    return request;
}

std::string RestServer::formatResponse(const HttpResponse& response) {
    DEBUG_FUNCTION_ENTER();
    
    std::ostringstream stream;
    
    // Format status line
    stream << "HTTP/1.1 " << response.statusCode << " " << response.statusMessage << "\r\n";
    
    // Format headers
    for (const auto& header : response.headers) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    
    // Add Content-Length header if not already present
    if (response.headers.find("Content-Length") == response.headers.end()) {
        stream << "Content-Length: " << response.body.length() << "\r\n";
    }
    
    // Add Date header if not already present
    if (response.headers.find("Date") == response.headers.end()) {
        stream << "Date: " << utils::StringUtils::getCurrentTimeString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
    }
    
    // Add Server header if not already present
    if (response.headers.find("Server") == response.headers.end()) {
        stream << "Server: D3Server/" << m_config->getServerConfig().serverName << "\r\n";
    }
    
    // End of headers
    stream << "\r\n";
    
    // Body
    stream << response.body;
    
    DEBUG_FUNCTION_EXIT();
    return stream.str();
}

void RestServer::registerRoutes() {
    DEBUG_FUNCTION_ENTER();
    
    std::lock_guard<std::mutex> lock(m_routesMutex);
    
    // Register account routes
    
    // Create account
    m_routes["POST"]["/api/accounts"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route doesn't require authentication as it's used to create new accounts
        if (m_accountController) {
            return m_accountController->createAccount(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Get all accounts
    m_routes["GET"]["/api/accounts"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            return m_accountController->getAllAccounts(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Get account by login
    m_routes["GET"]["/api/accounts/{login}"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->getAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Update account
    m_routes["PUT"]["/api/accounts/{login}"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->updateAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Delete account
    m_routes["DELETE"]["/api/accounts/{login}"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->deleteAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Ban account
    m_routes["POST"]["/api/accounts/{login}/ban"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)/ban");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_adminController->banAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Get character list for account
    m_routes["GET"]["/api/accounts/{login}/characters"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)/characters");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->getCharacters(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Create character for account
    m_routes["POST"]["/api/accounts/{login}/characters"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)/characters");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->createCharacter(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Admin routes
    
    // Server status
    m_routes["GET"]["/api/server/status"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->getServerStatus(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Server shutdown
    m_routes["POST"]["/api/server/shutdown"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->shutdownServer(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Server restart
    m_routes["POST"]["/api/server/restart"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->restartServer(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Get logs
    m_routes["GET"]["/api/logs"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->getLogs(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    LOG_INFO("REST API routes registered");
    DEBUG_FUNCTION_EXIT();
}

HttpResponse RestServer::createJsonResponse(int statusCode, const std::string& body) {
    DEBUG_FUNCTION_ENTER();
    
    HttpResponse response;
    response.statusCode = statusCode;
    
    switch (statusCode) {
        case 200: response.statusMessage = "OK"; break;
        case 201: response.statusMessage = "Created"; break;
        case 204: response.statusMessage = "No Content"; break;
        case 400: response.statusMessage = "Bad Request"; break;
        case 401: response.statusMessage = "Unauthorized"; break;
        case 403: response.statusMessage = "Forbidden"; break;
        case 404: response.statusMessage = "Not Found"; break;
        case 409: response.statusMessage = "Conflict"; break;
        case 500: response.statusMessage = "Internal Server Error"; break;
        default: response.statusMessage = "Unknown"; break;
    }
    
    response.headers["Content-Type"] = "application/json";
    response.body = body;
    
    DEBUG_FUNCTION_EXIT();
    return response;
}

HttpResponse RestServer::handleUnauthorized() {
    DEBUG_FUNCTION_ENTER();
    
    HttpResponse response = createJsonResponse(401, "{\"error\":\"Unauthorized\",\"message\":\"Authentication required\"}");
    response.headers["WWW-Authenticate"] = "Bearer";
    
    DEBUG_FUNCTION_EXIT();
    return response;
}

bool RestServer::isAuthorized(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    
    // Get API key from request
    std::string apiKey = getApiKey(request);
    
    // Validate API key
    bool authorized = !apiKey.empty() && validateApiKey(apiKey);
    
    DEBUG_FUNCTION_EXIT();
    return authorized;
}

std::string RestServer::getApiKey(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    
    // Check Authorization header
    auto it = request.headers.find("Authorization");
    if (it != request.headers.end()) {
        std::string authHeader = it->second;
        
        // Check if it's a Bearer token
        if (utils::StringUtils::startsWith(authHeader, "Bearer ")) {
            DEBUG_FUNCTION_EXIT();
            return authHeader.substr(7); // Skip "Bearer "
        }
    }
    
    // Check for API key in query parameters
    auto queryPos = request.uri.find('?');
    if (queryPos != std::string::npos) {
        std::string queryString = request.uri.substr(queryPos + 1);
        std::vector<std::string> pairs = utils::StringUtils::split(queryString, '&');
        
        for (const auto& pair : pairs) {
            std::vector<std::string> keyValue = utils::StringUtils::split(pair, '=');
            if (keyValue.size() == 2 && keyValue[0] == "api_key") {
                DEBUG_FUNCTION_EXIT();
                return keyValue[1];
            }
        }
    }
    
    DEBUG_FUNCTION_EXIT();
    return "";
}

bool RestServer::validateApiKey(const std::string& apiKey) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(apiKey);
    
    // TODO: Implement proper API key validation against database
    // For now, just check if it's not empty and meets a minimum length requirement
    bool valid = !apiKey.empty() && apiKey.length() >= 32;
    
    DEBUG_FUNCTION_EXIT();
    return valid;
}

} // namespace rest_api
} // namespace d3server
```

Now, let's create the AccountController class that handles account-related operations through the REST API:

```cpp
#pragma once

#include "rest_api/rest_server.h"

#include <memory>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace rest_api {

/**
 * @brief Controller for account-related API endpoints
 */
class AccountController {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    AccountController(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~AccountController();
    
    /**
     * @brief Create a new account
     * @param request HTTP request containing account data
     * @return HTTP response
     */
    HttpResponse createAccount(const HttpRequest& request);
    
    /**
     * @brief Get all accounts
     * @param request HTTP request
     * @return HTTP response containing a list of accounts
     */
    HttpResponse getAllAccounts(const HttpRequest& request);
    
    /**
     * @brief Get account by login
     * @param request HTTP request
     * @param login Account login
     * @return HTTP response containing account data
     */
    HttpResponse getAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Update an account
     * @param request HTTP request containing updated account data
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse updateAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Delete an account
     * @param request HTTP request
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse deleteAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Get characters for an account
     * @param request HTTP request
     * @param login Account login
     * @return HTTP response containing a list of characters
     */
    HttpResponse getCharacters(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Create a new character for an account
     * @param request HTTP request containing character data
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse createCharacter(const HttpRequest& request, const std::string& login);
    
private:
    /**
     * @brief Helper to validate account data
     * @param json Account data as JSON
     * @param errorMessage Output variable for validation error message
     * @return True if the account data is valid, false otherwise
     */
    bool validateAccountData(const std::string& json, std::string& errorMessage);
    
    /**
     * @brief Helper to validate character data
     * @param json Character data as JSON
     * @param errorMessage Output variable for validation error message
     * @return True if the character data is valid, false otherwise
     */
    bool validateCharacterData(const std::string& json, std::string& errorMessage);
    
    /**
     * @brief Parse JSON from request body
     * @param request HTTP request
     * @param errorResponse Output variable for error response
     * @return JSON string if valid, empty string if invalid
     */
    std::string parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse);
    
    /**
     * @brief Create a JSON error response
     * @param statusCode HTTP status code
     * @param message Error message
     * @return HTTP response
     */
    HttpResponse createErrorResponse(int statusCode, const std::string& message);
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
};

} // namespace rest_api
} // namespace d3server
```

Now let's implement the AccountController class:

```cpp
#include "rest_api/account_controller.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "database/database_manager.h"

#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

namespace d3server {
namespace rest_api {

AccountController::AccountController(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AccountController created");
    DEBUG_FUNCTION_EXIT();
}

AccountController::~AccountController() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AccountController destroyed");
    DEBUG_FUNCTION_EXIT();
}

HttpResponse AccountController::createAccount(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    // Validate account data
    std::string errorMessage;
    if (!validateAccountData(jsonStr, errorMessage)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, errorMessage);
    }
    
    try {
        // Parse account data
        json accountData = json::parse(jsonStr);
        
        // Extract required fields
        std::string login = accountData["login"];
        std::string email = accountData["email"];
        std::string password = accountData["password"];
        
        // Check if account already exists
        if (m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(409, "Account with this login already exists");
        }
        
        // Create account
        bool success = m_dbManager->createAccount(login, email, password);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to create account");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 201; // Created
        response.statusMessage = "Created";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Account created successfully"})";
        
        LOG_INFO("Account created: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception creating account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::getAllAccounts(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    try {
        // Get accounts from database
        std::vector<database::Account> accounts = m_dbManager->getAllAccounts();
        
        // Convert to JSON
        json response = json::array();
        for (const auto& account : accounts) {
            json accountJson = {
                {"login", account.login},
                {"email", account.email},
                {"created", account.created},
                {"last_login", account.lastLogin},
                {"banned", account.banned},
                {"character_count", account.characterCount}
            };
            response.push_back(accountJson);
        }
        
        // Return response
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = response.dump(4); // Pretty print with 4 spaces indentation
        
        DEBUG_FUNCTION_EXIT();
        return httpResponse;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception getting all accounts: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::getAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Get account from database
        database::Account account = m_dbManager->getAccount(login);
        
        // Convert to JSON
        json response = {
            {"login", account.login},
            {"email", account.email},
            {"created", account.created},
            {"last_login", account.lastLogin},
            {"banned", account.banned},
            {"character_count", account.characterCount}
        };
        
        // Return response
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = response.dump(4); // Pretty print with 4 spaces indentation
        
        DEBUG_FUNCTION_EXIT();
        return httpResponse;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception getting account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::updateAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    // Check if account exists
    if (!m_dbManager->accountExists(login)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(404, "Account not found");
    }
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Parse account data
        json accountData = json::parse(jsonStr);
        
        // Prepare update data - only email and password can be updated
        std::string newEmail = accountData.contains("email") ? accountData["email"] : "";
        std::string newPassword = accountData.contains("password") ? accountData["password"] : "";
        
        // Validate email if provided
        if (!newEmail.empty()) {
            // Simple email validation with regex
            std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
            if (!std::regex_match(newEmail, emailRegex)) {
                DEBUG_FUNCTION_EXIT();
                return createErrorResponse(400, "Invalid email format");
            }
        }
        
        // Validate password if provided
        if (!newPassword.empty() && newPassword.length() < 8) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(400, "Password must be at least 8 characters long");
        }
        
        // Update account
        bool success = m_dbManager->updateAccount(login, newEmail, newPassword);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to update account");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Account updated successfully"})";
        
        LOG_INFO("Account updated: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception updating account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::deleteAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Delete account
        bool success = m_dbManager->deleteAccount(login);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to delete account");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Account deleted successfully"})";
        
        LOG_INFO("Account deleted: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception deleting account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::getCharacters(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Get characters from database
        std::vector<database::Character> characters = m_dbManager->getCharacters(login);
        
        // Convert to JSON
        json response = json::array();
        for (const auto& character : characters) {
            json characterJson = {
                {"id", character.id},
                {"name", character.name},
                {"class_id", character.classId},
                {"level", character.level},
                {"created", character.created},
                {"last_played", character.lastPlayed}
            };
            response.push_back(characterJson);
        }
        
        // Return response
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = response.dump(4); // Pretty print with 4 spaces indentation
        
        DEBUG_FUNCTION_EXIT();
        return httpResponse;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception getting characters: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::createCharacter(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    // Check if account exists
    if (!m_dbManager->accountExists(login)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(404, "Account not found");
    }
    
    // Check character limit
    if (m_dbManager->getCharacterCount(login) >= m_config->getServerConfig().maxCharactersPerAccount) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, "Maximum number of characters reached for this account");
    }
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    // Validate character data
    std::string errorMessage;
    if (!validateCharacterData(jsonStr, errorMessage)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, errorMessage);
    }
    
    try {
        // Parse character data
        json characterData = json::parse(jsonStr);
        
        // Extract required fields
        std::string name = characterData["name"];
        int classId = characterData["class_id"];
        
        // Check if character name already exists
        if (m_dbManager->characterExists(name)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(409, "Character with this name already exists");
        }
        
        // Create character
        database::Character character;
        character.name = name;
        character.classId = classId;
        character.level = 1; // Start at level 1
        
        bool success = m_dbManager->createCharacter(login, character);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to create character");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 201; // Created
        response.statusMessage = "Created";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Character created successfully"})";
        
        LOG_INFO("Character created: " + name + " for account: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception creating character: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

bool AccountController::validateAccountData(const std::string& jsonStr, std::string& errorMessage) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        json accountData = json::parse(jsonStr);
        
        // Check required fields
        if (!accountData.contains("login")) {
            errorMessage = "Missing required field: login";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        if (!accountData.contains("email")) {
            errorMessage = "Missing required field: email";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        if (!accountData.contains("password")) {
            errorMessage = "Missing required field: password";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate login
        std::string login = accountData["login"];
        if (login.empty() || login.length() < 3 || login.length() > 16) {
            errorMessage = "Login must be between 3 and 16 characters";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate login characters (alphanumeric)
        std::regex loginRegex("^[a-zA-Z0-9]+$");
        if (!std::regex_match(login, loginRegex)) {
            errorMessage = "Login can only contain alphanumeric characters";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate email
        std::string email = accountData["email"];
        std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        if (!std::regex_match(email, emailRegex)) {
            errorMessage = "Invalid email format";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate password
        std::string password = accountData["password"];
        if (password.length() < 8) {
            errorMessage = "Password must be at least 8 characters long";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const json::exception& e) {
        errorMessage = "Invalid JSON: " + std::string(e.what());
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

bool AccountController::validateCharacterData(const std::string& jsonStr, std::string& errorMessage) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        json characterData = json::parse(jsonStr);
        
        // Check required fields
        if (!characterData.contains("name")) {
            errorMessage = "Missing required field: name";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        if (!characterData.contains("class_id")) {
            errorMessage = "Missing required field: class_id";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate name
        std::string name = characterData["name"];
        if (name.empty() || name.length() < 2 || name.length() > 12) {
            errorMessage = "Name must be between 2 and 12 characters";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate name characters (alphanumeric and some special chars)
        std::regex nameRegex("^[a-zA-Z0-9_-]+$");
        if (!std::regex_match(name, nameRegex)) {
            errorMessage = "Name can only contain alphanumeric characters, underscores, and hyphens";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate class_id
        int classId = characterData["class_id"];
        if (classId < 1 || classId > 7) { // Assuming 7 classes in Diablo 3
            errorMessage = "Invalid class_id. Must be between 1 and 7";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const json::exception& e) {
        errorMessage = "Invalid JSON: " + std::string(e.what());
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

std::string AccountController::parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse) {
    DEBUG_FUNCTION_ENTER();
    
    // Check Content-Type header
    auto contentTypeIt = request.headers.find("Content-Type");
    if (contentTypeIt == request.headers.end() || contentTypeIt->second != "application/json") {
        errorResponse = createErrorResponse(400, "Content-Type must be application/json");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    // Check body
    if (request.body.empty()) {
        errorResponse = createErrorResponse(400, "Request body is empty");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    // Validate JSON syntax
    try {
        json::parse(request.body);
    }
    catch (const json::exception& e) {
        errorResponse = createErrorResponse(400, "Invalid JSON: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    DEBUG_FUNCTION_EXIT();
    return request.body;
}

HttpResponse AccountController::createErrorResponse(int statusCode, const std::string& message) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(statusCode);
    DEBUG_VARIABLE(message);
    
    HttpResponse response;
    response.statusCode = statusCode;
    
    switch (statusCode) {
        case 400: response.statusMessage = "Bad Request"; break;
        case 401: response.statusMessage = "Unauthorized"; break;
        case 403: response.statusMessage = "Forbidden"; break;
        case 404: response.statusMessage = "Not Found"; break;
        case 409: response.statusMessage = "Conflict"; break;
        case 500: response.statusMessage = "Internal Server Error"; break;
        default: response.statusMessage = "Error"; break;
    }
    
    response.headers["Content-Type"] = "application/json";
    
    // Create error JSON
    json error = {
        {"error", true},
        {"message", message}
    };
    
    response.body = error.dump();
    
    DEBUG_FUNCTION_EXIT();
    return response;
}

} // namespace rest_api
} // namespace d3server
```

Now let's create the AdminController class for administrative operations:

```cpp
#pragma once

#include "rest_api/rest_server.h"

#include <memory>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace rest_api {

/**
 * @brief Controller for administrative API endpoints
 */
class AdminController {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    AdminController(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~AdminController();
    
    /**
     * @brief Ban/unban an account
     * @param request HTTP request containing ban data
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse banAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Get server status
     * @param request HTTP request
     * @return HTTP response containing server status
     */
    HttpResponse getServerStatus(const HttpRequest& request);
    
    /**
     * @brief Shutdown the server
     * @param request HTTP request
     * @return HTTP response
     */
    HttpResponse shutdownServer(const HttpRequest& request);
    
    /**
     * @brief Restart the server
     * @param request HTTP request
     * @return HTTP response
     */
    HttpResponse restartServer(const HttpRequest& request);
    
    /**
     * @brief Get server logs
     * @param request HTTP request
     * @return HTTP response containing logs
     */
    HttpResponse getLogs(const HttpRequest& request);
    
private:
    /**
     * @brief Parse JSON from request body
     * @param request HTTP request
     * @param errorResponse Output variable for error response
     * @return JSON string if valid, empty string if invalid
     */
    std::string parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse);
    
    /**
     * @brief Create a JSON error response
     * @param statusCode HTTP status code
     * @param message Error message
     * @return HTTP response
     */
    HttpResponse createErrorResponse(int statusCode, const std::string& message);
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
};

} // namespace rest_api
} // namespace d3server
```

Let's implement the AdminController class:

```cpp
#include "rest_api/admin_controller.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "database/database_manager.h"

#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

namespace d3server {
namespace rest_api {

AdminController::AdminController(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AdminController created");
    DEBUG_FUNCTION_EXIT();
}

AdminController::~AdminController() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AdminController destroyed");
    DEBUG_FUNCTION_EXIT();
}

HttpResponse AdminController::banAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Parse ban data
        json banData = json::parse(jsonStr);
        
        // Extract ban status
        bool banned = banData.contains("banned") ? banData["banned"].get<bool>() : true;
        std::string reason = banData.contains("reason") ? banData["reason"].get<std::string>() : "";
        
        // Ban or unban account
        bool success = m_dbManager->setBanStatus(login, banned, reason);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to update ban status");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        
        if (banned) {
            response.body = R"({"success":true,"message":"Account banned successfully"})";
            LOG_INFO("Account banned: " + login + ", reason: " + reason);
        } else {
            response.body = R"({"success":true,"message":"Account unbanned successfully"})";
            LOG_INFO("Account unbanned: " + login);
        }
        
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception updating ban status: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::getServerStatus(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    try {
        // Get system stats
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream timeStr;
        timeStr << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        
        // Get connected clients/stats from database
        int numAccounts = m_dbManager->getAccountCount();
        int numCharacters = m_dbManager->getTotalCharacterCount();
        int numOnlinePlayers = m_dbManager->getOnlinePlayerCount();
        
        // Create JSON response
        json response = {
            {"server_name", m_config->getServerConfig().serverName},
            {"version", "1.0.0"}, // TODO: Get from version file
            {"uptime", "Unknown"}, // TODO: Calculate uptime
            {"current_time", timeStr.str()},
            {"server_stats", {
                {"accounts", numAccounts},
                {"characters", numCharacters},
                {"online_players", numOnlinePlayers},
                {"max_players", m_config->getServerConfig().maxPlayers}
            }},
            {"system_info", {
                {"os", "Linux"}, // TODO: Get OS info
                {"cpu_usage", 0.0}, // TODO: Get CPU usage
                {"memory_usage", 0.0}, // TODO: Get memory usage
                {"debug_enabled", m_config->getServerConfig().enableDebug}
            }}
        };
        
        // Return response
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = response.dump(4); // Pretty print with 4 spaces indentation
        
        DEBUG_FUNCTION_EXIT();
        return httpResponse;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception getting server status: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::shutdownServer(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Parse shutdown data
        json shutdownData = json::parse(jsonStr);
        
        // Extract delay (optional)
        int delay = shutdownData.contains("delay") ? shutdownData["delay"].get<int>() : 0;
        std::string reason = shutdownData.contains("reason") ? shutdownData["reason"].get<std::string>() : "Server maintenance";
        
        // Log shutdown request
        LOG_INFO("Server shutdown requested. Reason: " + reason + ", Delay: " + std::to_string(delay) + " seconds");
        
        // Start shutdown in a separate thread
        if (delay > 0) {
            std::thread([delay, reason]() {
                LOG_INFO("Server will shut down in " + std::to_string(delay) + " seconds. Reason: " + reason);
                std::this_thread::sleep_for(std::chrono::seconds(delay));
                LOG_INFO("Shutting down now...");
                // TODO: Signal main server to shutdown
                // For now, just exit
                exit(0);
            }).detach();
        } else {
            std::thread([]() {
                LOG_INFO("Shutting down now...");
                // A small delay to allow the response to be sent
                std::this_thread::sleep_for(std::chrono::seconds(1));
                // TODO: Signal main server to shutdown
                // For now, just exit
                exit(0);
            }).detach();
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        
        if (delay > 0) {
            response.body = "{\"success\":true,\"message\":\"Server will shut down in " + std::to_string(delay) + " seconds\"}";
        } else {
            response.body = R"({"success":true,"message":"Server is shutting down"})";
        }
        
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during server shutdown: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::restartServer(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Parse restart data
        json restartData = json::parse(jsonStr);
        
        // Extract delay (optional)
        int delay = restartData.contains("delay") ? restartData["delay"].get<int>() : 0;
        std::string reason = restartData.contains("reason") ? restartData["reason"].get<std::string>() : "Server maintenance";
        
        // Log restart request
        LOG_INFO("Server restart requested. Reason: " + reason + ", Delay: " + std::to_string(delay) + " seconds");
        
        // Start restart in a separate thread
        if (delay > 0) {
            std::thread([delay, reason]() {
                LOG_INFO("Server will restart in " + std::to_string(delay) + " seconds. Reason: " + reason);
                std::this_thread::sleep_for(std::chrono::seconds(delay));
                LOG_INFO("Restarting now...");
                // TODO: Signal main server to restart
                // For now, just exit (a service manager would restart the server)
                exit(0);
            }).detach();
        } else {
            std::thread([]() {
                LOG_INFO("Restarting now...");
                // A small delay to allow the response to be sent
                std::this_thread::sleep_for(std::chrono::seconds(1));
                // TODO: Signal main server to restart
                // For now, just exit (a service manager would restart the server)
                exit(0);
            }).detach();
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        
        if (delay > 0) {
            response.body = "{\"success\":true,\"message\":\"Server will restart in " + std::to_string(delay) + " seconds\"}";
        } else {
            response.body = R"({"success":true,"message":"Server is restarting"})";
        }
        
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during server restart: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::getLogs(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    try {
        // Parse query parameters
        std::string logLevel = "INFO"; // Default log level
        int lines = 100; // Default number of lines
        
        // Extract query parameters
        auto queryPos = request.uri.find('?');
        if (queryPos != std::string::npos) {
            std::string queryString = request.uri.substr(queryPos + 1);
            std::vector<std::string> pairs = utils::StringUtils::split(queryString, '&');
            
            for (const auto& pair : pairs) {
                std::vector<std::string> keyValue = utils::StringUtils::split(pair, '=');
                if (keyValue.size() == 2) {
                    if (keyValue[0] == "level") {
                        logLevel = keyValue[1];
                    } else if (keyValue[0] == "lines") {
                        try {
                            lines = std::stoi(keyValue[1]);
                            if (lines <= 0) lines = 100;
                            if (lines > 1000) lines = 1000; // Limit to 1000 lines
                        } catch (...) {
                            lines = 100;
                        }
                    }
                }
            }
        }
        
        // Get log file path
        std::string logPath = m_config->getServerConfig().logPath;
        
        // Check if file exists
        if (!utils::FileUtils::fileExists(logPath)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Log file not found");
        }
        
        // Read log file
        std::vector<std::string> allLines;
        bool success = utils::FileUtils::readFileLines(logPath, allLines);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to read log file");
        }
        
        // Filter by log level
        std::vector<std::string> filteredLines;
        for (const auto& line : allLines) {
            // Simple log level filtering (assumes log format like "[INFO]")
            if (logLevel == "ALL" || line.find("[" + logLevel + "]") != std::string::npos) {
                filteredLines.push_back(line);
            }
        }
        
        // Get the last 'lines' lines
        size_t startLine = 0;
        if (filteredLines.size() > static_cast<size_t>(lines)) {
            startLine = filteredLines.size() - static_cast<size_t>(lines);
        }
        
        std::vector<std::string> resultLines(filteredLines.begin() + startLine, filteredLines.end());
        
        // Create JSON response
        json response = {
            {"log_level", logLevel},
            {"total_lines", filteredLines.size()},
            {"returned_lines", resultLines.size()},
            {"logs", resultLines}
        };
        
        // Return response
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = response.dump(4); // Pretty print with 4 spaces indentation
        
        DEBUG_FUNCTION_EXIT();
        return httpResponse;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception getting logs: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

std::string AdminController::parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse) {
    DEBUG_FUNCTION_ENTER();
    
    // Check Content-Type header
    auto contentTypeIt = request.headers.find("Content-Type");
    if (contentTypeIt == request.headers.end() || contentTypeIt->second != "application/json") {
        errorResponse = createErrorResponse(400, "Content-Type must be application/json");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    // Check body
    if (request.body.empty()) {
        errorResponse = createErrorResponse(400, "Request body is empty");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    // Validate JSON syntax
    try {
        json::parse(request.body);
    }
    catch (const json::exception& e) {
        errorResponse = createErrorResponse(400, "Invalid JSON: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    DEBUG_FUNCTION_EXIT();
    return request.body;
}

HttpResponse AdminController::createErrorResponse(int statusCode, const std::string& message) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(statusCode);
    DEBUG_VARIABLE(message);
    
    HttpResponse response;
    response.statusCode = statusCode;
    
    switch (statusCode) {
        case 400: response.statusMessage = "Bad Request"; break;
        case 401: response.statusMessage = "Unauthorized"; break;
        case 403: response.statusMessage = "Forbidden"; break;
        case 404: response.statusMessage = "Not Found"; break;
        case 409: response.statusMessage = "Conflict"; break;
        case 500: response.statusMessage = "Internal Server Error"; break;
        default: response.statusMessage = "Error"; break;
    }
    
    response.headers["Content-Type"] = "application/json";
    
    // Create error JSON
    json error = {
        {"error", true},
        {"message", message}
    };
    
    response.body = error.dump();
    
    DEBUG_FUNCTION_EXIT();
    return response;
}

} // namespace rest_api
} // namespace d3server
```

Now let's create the REST API documentation in the docs directory:

```markdown
# D3Server REST API Documentation

This document describes the REST API endpoints available in the D3Server.

## Authentication

Most API endpoints require authentication. Authentication is performed using a Bearer token in the Authorization header.

```
Authorization: Bearer your_api_key
```

Alternatively, you can provide the API key as a query parameter:

```
?api_key=your_api_key
```

## API Endpoints

### Account Management

#### Create Account

Create a new user account.

**URL**: `/api/accounts`

**Method**: `POST`

**Auth required**: No

**Request Body**:
```json
{
    "login": "username",
    "email": "user@example.com",
    "password": "password123"
}
```

**Success Response**:
- **Code**: 201 Created
- **Content**:
```json
{
    "success": true,
    "message": "Account created successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 409 Conflict (Account already exists)
- **Code**: 500 Internal Server Error

#### Get All Accounts

Get a list of all accounts.

**URL**: `/api/accounts`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
[
    {
        "login": "username1",
        "email": "user1@example.com",
        "created": "2023-01-01 12:00:00",
        "last_login": "2023-01-02 12:00:00",
        "banned": false,
        "character_count": 2
    },
    {
        "login": "username2",
        "email": "user2@example.com",
        "created": "2023-01-03 12:00:00",
        "last_login": "2023-01-04 12:00:00",
        "banned": false,
        "character_count": 1
    }
]
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Get Account

Get information about a specific account.

**URL**: `/api/accounts/{login}`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "login": "username",
    "email": "user@example.com",
    "created": "2023-01-01 12:00:00",
    "last_login": "2023-01-02 12:00:00",
    "banned": false,
    "character_count": 2
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Update Account

Update an existing account.

**URL**: `/api/accounts/{login}`

**Method**: `PUT`

**Auth required**: Yes

**Request Body**:
```json
{
    "email": "newemail@example.com",
    "password": "newpassword123"
}
```
Note: Both fields are optional. Include only the fields you want to update.

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Account updated successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Delete Account

Delete an account.

**URL**: `/api/accounts/{login}`

**Method**: `DELETE`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Account deleted successfully"
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Ban Account

Ban or unban an account.

**URL**: `/api/accounts/{login}/ban`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "banned": true,
    "reason": "Cheating"
}
```

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Account banned successfully"
}
```
or
```json
{
    "success": true,
    "message": "Account unbanned successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

### Character Management

#### Get All Characters

Get a list of all characters for an account.

**URL**: `/api/accounts/{login}/characters`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
[
    {
        "id": 1,
        "name": "Barbarian1",
        "class_id": 1,
        "level": 60,
        "created": "2023-01-01 12:00:00",
        "last_played": "2023-01-02 12:00:00"
    },
    {
        "id": 2,
        "name": "Wizard1",
        "class_id": 3,
        "level": 45,
        "created": "2023-01-03 12:00:00",
        "last_played": "2023-01-04 12:00:00"
    }
]
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Create Character

Create a new character for an account.

**URL**: `/api/accounts/{login}/characters`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "name": "MyCharacter",
    "class_id": 2
}
```

Class IDs:
1. Barbarian
2. Monk
3. Wizard
4. Witch Doctor
5. Demon Hunter
6. Crusader
7. Necromancer

**Success Response**:
- **Code**: 201 Created
- **Content**:
```json
{
    "success": true,
    "message": "Character created successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data or max character limit reached)
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found (Account not found)
- **Code**: 409 Conflict (Character name already exists)
- **Code**: 500 Internal Server Error

### Server Management

#### Get Server Status

Get current server status.

**URL**: `/api/server/status`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "server_name": "D3Server",
    "version": "1.0.0",
    "uptime": "3 days, 2 hours, 15 minutes",
    "current_time": "2023-01-01 12:00:00",
    "server_stats": {
        "accounts": 150,
        "characters": 432,
        "online_players": 25,
        "max_players": 1000
    },
    "system_info": {
        "os": "Linux",
        "cpu_usage": 15.2,
        "memory_usage": 256.5,
        "debug_enabled": true
    }
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Server Shutdown

Initiate server shutdown.

**URL**: `/api/server/shutdown`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "delay": 60,
    "reason": "Server maintenance"
}
```
Note: Delay is in seconds, defaults to 0 (immediate) if not provided.

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Server will shut down in 60 seconds"
}
```
or
```json
{
    "success": true,
    "message": "Server is shutting down"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Server Restart

Initiate server restart.

**URL**: `/api/server/restart`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "delay": 60,
    "reason": "Server update"
}
```
Note: Delay is in seconds, defaults to 0 (immediate) if not provided.

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Server will restart in 60 seconds"
}
```
or
```json
{
    "success": true,
    "message": "Server is restarting"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Get Logs

Get server logs.

**URL**: `/api/logs`

**Method**: `GET`

**Auth required**: Yes

**Query Parameters**:
- `level` (optional): Log level to filter (e.g., INFO, WARNING, ERROR, ALL). Default: INFO
- `lines` (optional): Number of log lines to retrieve. Default: 100, Max: 1000

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "log_level": "INFO",
    "total_lines": 1500,
    "returned_lines": 100,
    "logs": [
        "[INFO] 2023-01-01 12:00:00 - Server started",
        "[INFO] 2023-01-01 12:01:00 - Player connected: username",
        "..."
    ]
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found (Log file not found)
- **Code**: 500 Internal Server Error

## Error Response Format

All API errors follow this format:

```json
{
    "error": true,
    "message": "Error message description"
}
```
```

Now let's create a basic structure for the Battle.net server component:

```cpp
#pragma once

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace battle_net {

/**
 * @brief Represents a client connection to the Battle.net server
 */
class BattleNetClient;

/**
 * @brief Battle.net server that handles authentication and game coordination
 */
class BattleNetServer {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    BattleNetServer(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~BattleNetServer();
    
    /**
     * @brief Initialize the Battle.net server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();
    
    /**
     * @brief Run the Battle.net server (blocking call)
     */
    void run();
    
    /**
     * @brief Shutdown the Battle.net server
     */
    void shutdown();
    
    /**
     * @brief Get the number of connected clients
     * @return Number of connected clients
     */
    size_t getClientCount() const;
    
    /**
     * @brief Is the server running
     * @return True if the server is running, false otherwise
     */
    bool isRunning() const;
    
private:
    /**
     * @brief Start accepting connections
     */
    void startAccept();
    
    /**
     * @brief Handle a new connection
     */
    void handleAccept(boost::asio::ip::tcp::socket socket);
    
    /**
     * @brief Process protocol commands
     */
    void processCommands();
    
    /**
     * @brief Remove disconnected clients
     */
    void cleanupClients();
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    std::atomic<bool> m_running;
    std::thread m_ioThread;
    
    mutable std::mutex m_clientsMutex;
    std::unordered_map<std::string, std::shared_ptr<BattleNetClient>> m_clients;
};

} // namespace battle_net
} // namespace d3server
```

Now let's define the BattleNetClient class:

```cpp
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace battle_net {

// Forward declarations
class BattleNetServer;

/**
 * @brief Represents the authentication state of a client
 */
enum class AuthState {
    NotAuthenticated,
    Authenticating,
    Authenticated,
    AuthFailed
};

/**
 * @brief Represents a client connection to the Battle.net server
 */
class BattleNetClient : public std::enable_shared_from_this<BattleNetClient> {
public:
    /**
     * @brief Constructor
     * @param socket Socket for the client connection
     * @param server Battle.net server instance
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    BattleNetClient(
        boost::asio::ip::tcp::socket socket,
        BattleNetServer& server,
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~BattleNetClient();
    
    /**
     * @brief Start processing client connection
     */
    void start();
    
    /**
     * @brief Send a message to the client
     * @param message Message to send
     */
    void sendMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Is the client connected
     * @return True if the client is connected, false otherwise
     */
    bool isConnected() const;
    
    /**
     * @brief Get the client's authentication state
     * @return Authentication state
     */
    AuthState getAuthState() const;
    
    /**
     * @brief Get the client's login
     * @return Login or empty string if not authenticated
     */
    std::string getLogin() const;
    
    /**
     * @brief Get the client's IP address
     * @return IP address
     */
    std::string getIpAddress() const;
    
    /**
     * @brief Get the time the client connected
     * @return Connection time
     */
    std::time_t getConnectTime() const;
    
    /**
     * @brief Get the last activity time
     * @return Last activity time
     */
    std::time_t getLastActivityTime() const;
    
private:
    /**
     * @brief Read data from the client
     */
    void readHeader();
    
    /**
     * @brief Read the message body
     * @param messageSize Size of the message to read
     */
    void readBody(uint32_t messageSize);
    
    /**
     * @brief Process a received message
     * @param message Message data
     */
    void processMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Update the last activity time
     */
    void updateLastActivityTime();
    
    /**
     * @brief Disconnect the client
     */
    void disconnect();
    
    /**
     * @brief Handle authentication
     * @param login Login
     * @param password Password
     * @return True if authentication succeeded, false otherwise
     */
    bool handleAuth(const std::string& login, const std::string& password);
    
    /**
     * @brief Write next message in the queue
     */
    void writeNextMessage();
    
    boost::asio::ip::tcp::socket m_socket;
    BattleNetServer& m_server;
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    std::atomic<bool> m_connected;
    std::atomic<AuthState> m_authState;
    std::string m_login;
    std::string m_ipAddress;
    std::time_t m_connectTime;
    std::time_t m_lastActivityTime;
    
    std::vector<uint8_t> m_headerBuffer;
    std::vector<uint8_t> m_bodyBuffer;
    
    std::mutex m_writeMutex;
    std::queue<std::vector<uint8_t>> m_writeQueue;
    bool m_writing;
};

} // namespace battle_net
} // namespace d3server
```

Now let's implement the BattleNet client and server:

```cpp
#include "battle_net/battle_net_server.h"
#include "battle_net/battle_net_client.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"

#include <functional>
#include <algorithm>
#include <chrono>

namespace d3server {
namespace battle_net {

BattleNetServer::BattleNetServer(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_ioContext(),
    m_acceptor(m_ioContext),
    m_running(false)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Battle.net server instance created");
    DEBUG_FUNCTION_EXIT();
}

BattleNetServer::~BattleNetServer() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("Battle.net server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool BattleNetServer::init() {
    DEBUG_FUNCTION_ENTER();
    
    try {
        LOG_INFO("Initializing Battle.net server...");
        
        // Initialize acceptor
        auto& networkConfig = m_config->getNetworkConfig();
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(networkConfig.bindIp),
            networkConfig.battleNetPort
        );
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
        
        LOG_INFO("Battle.net server initialized on " + networkConfig.bindIp + ":" + std::to_string(networkConfig.battleNetPort));
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during Battle.net server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void BattleNetServer::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("Battle.net server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    
    LOG_INFO("Starting Battle.net server...");
    
    // Start accepting connections
    startAccept();
    
    // Run the IO context
    m_ioThread = std::thread([this]() {
        DEBUG_CONTEXT("Battle.net server IO thread started");
        
        try {
            m_ioContext.run();
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception in Battle.net server IO thread: " + std::string(e.what()));
        }
        
        DEBUG_LOG("Battle.net server IO thread exited");
    });
    
    LOG_INFO("Battle.net server started");
    
    // Run the command processing loop
    while (m_running) {
        // Process commands from clients
        processCommands();
        
        // Clean up disconnected clients
        cleanupClients();
        
        // Sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetServer::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("Battle.net server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down Battle.net server...");
    m_running = false;
    
    // Stop the IO context
    m_ioContext.stop();
    
    // Wait for the IO thread to finish
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
    
    // Disconnect all clients
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.clear();
    }
    
    LOG_INFO("Battle.net server shutdown complete");
    DEBUG_FUNCTION_EXIT();
}

size_t BattleNetServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    return m_clients.size();
}

bool BattleNetServer::isRunning() const {
    return m_running;
}

void BattleNetServer::startAccept() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        DEBUG_LOG("Battle.net server is not running, not accepting connections");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                // Handle the new connection
                handleAccept(std::move(socket));
            } else {
                DEBUG_LOG("Error accepting connection: " + ec.message());
            }
            
            // Accept the next connection
            startAccept();
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetServer::handleAccept(boost::asio::ip::tcp::socket socket) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        // Get client IP address
        std::string clientIp = socket.remote_endpoint().address().to_string();
        LOG_INFO("New connection from " + clientIp);
        
        // Check if client IP is already connected too many times
        size_t connectionCount = 0;
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (const auto& client : m_clients) {
                if (client.second->getIpAddress() == clientIp) {
                    connectionCount++;
                }
            }
        }
        
        // Check if IP has reached max connections limit
        if (connectionCount >= static_cast<size_t>(m_config->getServerConfig().maxAccountsPerIp)) {
            LOG_WARNING("Connection from " + clientIp + " rejected (too many connections)");
            socket.close();
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Create a new client session
        auto client = std::make_shared<BattleNetClient>(
            std::move(socket),
            *this,
            m_config,
            m_dbManager
        );
        
        // Add client to the list
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clients[client->getIpAddress()] = client;
        }
        
        // Start processing client
        client->start();
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception handling new connection: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetServer::processCommands() {
    // This would process any pending commands from clients
    // For simplicity, we're not implementing actual command processing here
}

void BattleNetServer::cleanupClients() {
    DEBUG_FUNCTION_ENTER();
    
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    // Find disconnected clients
    std::vector<std::string> disconnectedClients;
    for (const auto& client : m_clients) {
        if (!client.second->isConnected()) {
            disconnectedClients.push_back(client.first);
        }
    }
    
    // Remove disconnected clients
    for (const auto& ip : disconnectedClients) {
        LOG_INFO("Removing disconnected client: " + ip);
        m_clients.erase(ip);
    }
    
    DEBUG_FUNCTION_EXIT();
}

} // namespace battle_net
} // namespace d3server
```

Let's implement the BattleNetClient class:

```cpp
#include "battle_net/battle_net_client.h"
#include "battle_net/battle_net_server.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/crypto_utils.h"
#include "database/database_manager.h"

#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>

namespace d3server {
namespace battle_net {

// Header size for Battle.net messages
constexpr size_t HEADER_SIZE = 4;

BattleNetClient::BattleNetClient(
    boost::asio::ip::tcp::socket socket,
    BattleNetServer& server,
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_socket(std::move(socket)),
    m_server(server),
    m_config(config),
    m_dbManager(dbManager),
    m_connected(true),
    m_authState(AuthState::NotAuthenticated),
    m_headerBuffer(HEADER_SIZE),
    m_writing(false)
{
    DEBUG_FUNCTION_ENTER();
    
    // Get client IP address
    try {
        m_ipAddress = m_socket.remote_endpoint().address().to_string();
    }
    catch (const std::exception& e) {
        m_ipAddress = "unknown";
        DEBUG_LOG("Failed to get client IP address: " + std::string(e.what()));
    }
    
    // Set client connect time
    m_connectTime = std::time(nullptr);
    m_lastActivityTime = m_connectTime;
    
    LOG_INFO("Battle.net client created from " + m_ipAddress);
    DEBUG_FUNCTION_EXIT();
}

BattleNetClient::~BattleNetClient() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_connected) {
        disconnect();
    }
    
    LOG_INFO("Battle.net client destroyed from " + m_ipAddress);
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::start() {
    DEBUG_FUNCTION_ENTER();
    
    // Start reading messages
    readHeader();
    
    // Send welcome message
    std::vector<uint8_t> welcomeMsg = { 0x01, 0x00, 0x00, 0x00, 0x00 };
    sendMessage(welcomeMsg);
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::sendMessage(const std::vector<uint8_t>& message) {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_LOG("Cannot send message, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    bool shouldWrite = false;
    
    {
        std::lock_guard<std::mutex> lock(m_writeMutex);
        shouldWrite = m_writeQueue.empty() && !m_writing;
        m_writeQueue.push(message);
    }
    
    if (shouldWrite) {
        writeNextMessage();
    }
    
    DEBUG_FUNCTION_EXIT();
}

bool BattleNetClient::isConnected() const {
    return m_connected;
}

AuthState BattleNetClient::getAuthState() const {
    return m_authState;
}

std::string BattleNetClient::getLogin() const {
    return m_login;
}

std::string BattleNetClient::getIpAddress() const {
    return m_ipAddress;
}

std::time_t BattleNetClient::getConnectTime() const {
    return m_connectTime;
}

std::time_t BattleNetClient::getLastActivityTime() const {
    return m_lastActivityTime;
}

void BattleNetClient::readHeader() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_LOG("Cannot read header, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    auto self = shared_from_this();
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_headerBuffer),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                // Update activity time
                updateLastActivityTime();
                
                // Extract message size from header
                uint32_t messageSize = 0;
                messageSize |= m_headerBuffer[0];
                messageSize |= (m_headerBuffer[1] << 8);
                messageSize |= (m_headerBuffer[2] << 16);
                messageSize |= (m_headerBuffer[3] << 24);
                
                // Check message size
                if (messageSize <= 0 || messageSize > 1024 * 1024) {
                    // Invalid message size, disconnect
                    LOG_WARNING("Invalid message size: " + std::to_string(messageSize));
                    disconnect();
                    return;
                }
                
                // Read message body
                readBody(messageSize);
            } else if (ec != boost::asio::error::operation_aborted) {
                if (ec == boost::asio::error::eof) {
                    LOG_INFO("Client disconnected: " + m_ipAddress);
                } else {
                    LOG_ERROR("Error reading header: " + ec.message());
                }
                disconnect();
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::readBody(uint32_t messageSize) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(messageSize);
    
    if (!m_connected) {
        DEBUG_LOG("Cannot read body, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    // Resize body buffer
    m_bodyBuffer.resize(messageSize);
    
    auto self = shared_from_this();
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_bodyBuffer),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                // Update activity time
                updateLastActivityTime();
                
                // Process message
                processMessage(m_bodyBuffer);
                
                // Start reading next message
                readHeader();
            } else if (ec != boost::asio::error::operation_aborted) {
                if (ec == boost::asio::error::eof) {
                    LOG_INFO("Client disconnected: " + m_ipAddress);
                } else {
                    LOG_ERROR("Error reading body: " + ec.message());
                }
                disconnect();
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::processMessage(const std::vector<uint8_t>& message) {
    DEBUG_FUNCTION_ENTER();
    
    if (message.empty()) {
        LOG_WARNING("Received empty message");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    // First byte is the message type
    uint8_t messageType = message[0];
    
    DEBUG_VARIABLE(messageType);
    
    // Handle message based on type
    switch (messageType) {
        case 0x01: // Authentication request
            if (message.size() >= 3) {
                // Simple authentication flow for demo
                // In a real implementation, this would be more complex
                
                // Extract login and password
                std::string login, password;
                
                // Parse login (assuming login length is at message[1])
                if (message.size() >= 2 + message[1]) {
                    login.assign(message.begin() + 2, message.begin() + 2 + message[1]);
                }
                
                // Parse password (assuming password length is after login)
                if (message.size() >= 3 + message[1] + message[2 + message[1]]) {
                    password.assign(
                        message.begin() + 3 + message[1],
                        message.begin() + 3 + message[1] + message[2 + message[1]]
                    );
                }
                
                LOG_INFO("Authentication request from " + m_ipAddress + ", login: " + login);
                
                // Update auth state and handle authentication
                m_authState = AuthState::Authenticating;
                
                if (handleAuth(login, password)) {
                    m_authState = AuthState::Authenticated;
                    m_login = login;
                    
                    // Send auth success
                    std::vector<uint8_t> authSuccessMsg = { 0x02, 0x01 };
                    sendMessage(authSuccessMsg);
                    
                    LOG_INFO("Authentication successful for " + login + " from " + m_ipAddress);
                } else {
                    m_authState = AuthState::AuthFailed;
                    
                    // Send auth failed
                    std::vector<uint8_t> authFailedMsg = { 0x02, 0x00 };
                    sendMessage(authFailedMsg);
                    
                    LOG_WARNING("Authentication failed for " + login + " from " + m_ipAddress);
                }
            } else {
                LOG_WARNING("Invalid authentication request");
            }
            break;
            
        case 0x03: // Ping
            // Send pong
            std::vector<uint8_t> pongMsg = { 0x04 };
            sendMessage(pongMsg);
            break;
            
        case 0x05: // Character list request
            if (m_authState == AuthState::Authenticated) {
                // In a real implementation, this would fetch characters from the database
                LOG_INFO("Character list requested by " + m_login);
                
                // For now, just send an empty character list
                std::vector<uint8_t> characterListMsg = { 0x06, 0x00, 0x00 };
                sendMessage(characterListMsg);
            } else {
                LOG_WARNING("Character list requested but client not authenticated");
                disconnect();
            }
            break;
            
        default:
            LOG_WARNING("Unknown message type: " + std::to_string(messageType));
            break;
    }
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::updateLastActivityTime() {
    m_lastActivityTime = std::time(nullptr);
}

void BattleNetClient::disconnect() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_connected = false;
    
    try {
        if (m_socket.is_open()) {
            boost::system::error_code ec;
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during client disconnect: " + std::string(e.what()));
    }
    
    LOG_INFO("Client disconnected: " + m_ipAddress);
    DEBUG_FUNCTION_EXIT();
}

bool BattleNetClient::handleAuth(const std::string& login, const std::string& password) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Check if account is banned
        if (m_dbManager->isAccountBanned(login)) {
            LOG_WARNING("Login attempt on banned account: " + login + " from " + m_ipAddress);
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Verify password
        bool success = m_dbManager->verifyAccountPassword(login, password);
        
        if (success) {
            // Update last login time
            m_dbManager->updateAccountLastLogin(login);
        }
        
        DEBUG_FUNCTION_EXIT();
        return success;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during authentication: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void BattleNetClient::writeNextMessage() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_LOG("Cannot write message, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_writeMutex);
    
    if (m_writeQueue.empty() || m_writing) {
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_writing = true;
    
    // Get next message
    std::vector<uint8_t> message = m_writeQueue.front();
    m_writeQueue.pop();
    
    // Prepare header (4 bytes, little-endian)
    std::vector<uint8_t> header(HEADER_SIZE);
    uint32_t messageSize = static_cast<uint32_t>(message.size());
    header[0] = messageSize & 0xFF;
    header[1] = (messageSize >> 8) & 0xFF;
    header[2] = (messageSize >> 16) & 0xFF;
    header[3] = (messageSize >> 24) & 0xFF;
    
    // Combine header and message
    std::vector<uint8_t> packet;
    packet.reserve(header.size() + message.size());
    packet.insert(packet.end(), header.begin(), header.end());
    packet.insert(packet.end(), message.begin(), message.end());
    
    // Write to socket
    auto self = shared_from_this();
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(packet),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            std::lock_guard<std::mutex> lock(m_writeMutex);
            
            m_writing = false;
            
            if (!ec) {
                // Write next message if any
                if (!m_writeQueue.empty()) {
                    writeNextMessage();
                }
            } else if (ec != boost::asio::error::operation_aborted) {
                LOG_ERROR("Error writing message: " + ec.message());
                disconnect();
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

} // namespace battle_net
} // namespace d3server
```

Now let's create the documentation for the Battle.net protocol:

```markdown
# D3Server Battle.net Protocol Documentation

This document describes the Battle.net protocol used for communication between the D3Server and clients.

## Overview

The Battle.net protocol is a binary protocol used for authentication, character management, and game coordination. It uses a simple message-based format with a header followed by a variable-length body.

## Message Format

Each message has the following format:

```
+----------------+----------------+
| Header (4 bytes)| Body (variable) |
+----------------+----------------+
```

### Header

The header is 4 bytes long and contains the length of the body in little-endian format.

```
+-------------------+-------------------+-------------------+-------------------+
| Length (byte 0)   | Length (byte 1)   | Length (byte 2)   | Length (byte 3)   |
+-------------------+-------------------+-------------------+-------------------+
```

### Body

The body contains the message data, with the first byte indicating the message type.

```
+-------------------+-------------------+
| Message Type      | Message Data      |
+-------------------+-------------------+
```

## Message Types

### Server to Client

| Type (Hex) | Description            | Format                                    |
|------------|------------------------|-------------------------------------------|
| 0x01       | Welcome                | [0x01] [Version (4 bytes)]                |
| 0x02       | Authentication Result  | [0x02] [Result (1 = Success, 0 = Fail)]   |
| 0x04       | Pong                   | [0x04]                                    |
| 0x06       | Character List         | [0x06] [Count (2 bytes)] [Character Data] |

#### Character Data Format

Each character entry in the character list has the following format:

```
+--------------------+-----------------+--------------+--------------+------------------+
| ID (4 bytes)       | Name Length     | Name         | Class (1 byte)| Level (1 byte)  |
+--------------------+-----------------+--------------+--------------+------------------+
```

### Client to Server

| Type (Hex) | Description            | Format                                   |
|------------|------------------------|------------------------------------------|
| 0x01       | Authentication Request | [0x01] [Login Len (1 byte)] [Login] [Password Len (1 byte)] [Password] |
| 0x03       | Ping                   | [0x03]                                   |
| 0x05       | Request Character List | [0x05]                                   |
| 0x07       | Create Character       | [0x07] [Name Len (1 byte)] [Name] [Class (1 byte)] |
| 0x09       | Enter Game             | [0x09] [Character ID (4 bytes)]          |

## Authentication Process

1. Server sends Welcome message (0x01) upon client connection.
2. Client sends Authentication Request (0x01) with login and password.
3. Server validates credentials and sends Authentication Result (0x02).
4. If authentication is successful, client can proceed to request character list, create characters, etc.

## Class IDs

| ID | Class Name    |
|----|---------------|
| 1  | Barbarian     |
| 2  | Monk          |
| 3  | Wizard        |
| 4  | Witch Doctor  |
| 5  | Demon Hunter  |
| 6  | Crusader      |
| 7  | Necromancer   |

## Ping/Pong Mechanism

To detect connection drops and maintain connectivity, clients should send periodic Ping messages (0x03) to the server. The server responds with a Pong message (0x04).

## Error Handling

When an error occurs during message processing, the server may disconnect the client. No explicit error messages are defined in the protocol. The client should interpret connection termination as an indication of an error.

## Example Message Flow

### Authentication

1. Server → Client: Welcome
   ```
   Header: 05 00 00 00
   Body:   01 01 00 00 00
   ```

2. Client → Server: Authentication Request
   ```
   Header: 10 00 00 00
   Body:   01 05 "alice" 08 "password"
   ```

3. Server → Client: Authentication Result (Success)
   ```
   Header: 02 00 00 00
   Body:   02 01
   ```

### Character List Request

1. Client → Server: Request Character List
   ```
   Header: 01 00 00 00
   Body:   05
   ```

2. Server → Client: Character List (2 characters)
   ```
   Header: 20 00 00 00
   Body:   06 02 00 01 00 00 00 08 "Barbarian" 01 60 02 00 00 00 05 "Wizard" 03 45
   ```

## Implementation Notes

- All multi-byte integer values are transmitted in little-endian format.
- Strings are transmitted with a length prefix followed by the raw string bytes (no null terminator).
- The server enforces a maximum message size (currently 1MB) to prevent memory exhaustion attacks.
- Clients should handle connection drops and implement reconnection logic.
- Authentication uses plaintext passwords in this simple implementation, but a real implementation should use a secure authentication mechanism.
```

Now let's create the basic structure for the game server component:

```cpp
#pragma once

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace game_server {

/**
 * @brief Represents a connected game client
 */
class GameClient;

/**
 * @brief Represents a game session
 */
class GameSession;

/**
 * @brief Game server that handles in-game communication and game logic
 */
class GameServer {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    GameServer(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~GameServer();
    
    /**
     * @brief Initialize the game server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();
    
    /**
     * @brief Run the game server (blocking call)
     */
    void run();
    
    /**
     * @brief Shutdown the game server
     */
    void shutdown();
    
    /**
     * @brief Get the number of connected clients
     * @return Number of connected clients
     */
    size_t getClientCount() const;
    
    /**
     * @brief Get the number of active game sessions
     * @return Number of active game sessions
     */
    size_t getSessionCount() const;
    
    /**
     * @brief Check if the server is running
     * @return True if the server is running, false otherwise
     */
    bool isRunning() const;
    
private:
    /**
     * @brief Start accepting connections
     */
    void startAccept();
    
    /**
     * @brief Handle a new connection
     * @param socket Socket for the new connection
     */
    void handleAccept(boost::asio::ip::tcp::socket socket);
    
    /**
     * @brief Update game sessions
     */
    void updateSessions();
    
    /**
     * @brief Clean up disconnected clients and empty sessions
     */
    void cleanupClientsAndSessions();
    
    /**
     * @brief Create a new game session
     * @return Shared pointer to the new game session
     */
    std::shared_ptr<GameSession> createGameSession();
    
    /**
     * @brief Get a game session by ID
     * @param sessionId Session ID
     * @return Shared pointer to the game session, or nullptr if not found
     */
    std::shared_ptr<GameSession> getGameSession(uint32_t sessionId);
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    std::atomic<bool> m_running;
    std::atomic<uint32_t> m_nextSessionId;
    std::thread m_ioThread;
    std::thread m_updateThread;
    
    mutable std::mutex m_clientsMutex;
    std::unordered_map<std::string, std::shared_ptr<GameClient>> m_clients;
    
    mutable std::mutex m_sessionsMutex;
    std::unordered_map<uint32_t, std::shared_ptr<GameSession>> m_sessions;
};

} // namespace game_server
} // namespace d3server
```

Let's create the GameSession class:

```cpp
#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <chrono>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace game_server {

// Forward declarations
class GameClient;

/**
 * @brief Represents a game difficulty level
 */
enum class GameDifficulty {
    Normal,
    Hard,
    Expert,
    Master,
    Torment1,
    Torment2,
    Torment3,
    Torment4,
    Torment5,
    Torment6
};

/**
 * @brief Represents a game mode
 */
enum class GameMode {
    Campaign,
    Adventure,
    Rift,
    GreaterRift
};

/**
 * @brief Represents a game session where players can join and play together
 */
class GameSession : public std::enable_shared_from_this<GameSession> {
public:
    /**
     * @brief Constructor
     * @param sessionId Unique session ID
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    GameSession(
        uint32_t sessionId,
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~GameSession();
    
    /**
     * @brief Initialize the game session
     * @return True if initialization succeeded, false otherwise
     */
    bool init();
    
    /**
     * @brief Update the game session
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);
    
    /**
     * @brief Add a client to the session
     * @param client Client to add
     * @return True if client was added, false otherwise
     */
    bool addClient(std::shared_ptr<GameClient> client);
    
    /**
     * @brief Remove a client from the session
     * @param client Client to remove
     */
    void removeClient(std::shared_ptr<GameClient> client);
    
    /**
     * @brief Check if the session is empty
     * @return True if the session has no clients, false otherwise
     */
    bool isEmpty() const;
    
    /**
     * @brief Get the session ID
     * @return Session ID
     */
    uint32_t getSessionId() const;
    
    /**
     * @brief Get the number of clients in the session
     * @return Number of clients
     */
    size_t getClientCount() const;
    
    /**
     * @brief Get the difficulty level
     * @return Difficulty level
     */
    GameDifficulty getDifficulty() const;
    
    /**
     * @brief Set the difficulty level
     * @param difficulty Difficulty level
     */
    void setDifficulty(GameDifficulty difficulty);
    
    /**
     * @brief Get the game mode
     * @return Game mode
     */
    GameMode getGameMode() const;
    
    /**
     * @brief Set the game mode
     * @param mode Game mode
     */
    void setGameMode(GameMode mode);
    
    /**
     * @brief Broadcast a message to all clients in the session
     * @param message Message to broadcast
     * @param excludeClient Client to exclude from broadcast (nullptr to send to all)
     */
    void broadcastMessage(const std::vector<uint8_t>& message, std::shared_ptr<GameClient> excludeClient = nullptr);
    
private:
    /**
     * @brief Handle a client message
     * @param client Client that sent the message
     * @param message Message data
     */
    void handleMessage(std::shared_ptr<GameClient> client, const std::vector<uint8_t>& message);
    
    /**
     * @brief Process game logic
     * @param deltaTime Time elapsed since last update
     */
    void processGameLogic(float deltaTime);
    
    uint32_t m_sessionId;
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    std::atomic<GameDifficulty> m_difficulty;
    std::atomic<GameMode> m_mode;
    std::chrono::steady_clock::time_point m_creationTime;
    
    mutable std::mutex m_clientsMutex;
    std::unordered_map<std::string, std::shared_ptr<GameClient>> m_clients;
};

} // namespace game_server
} // namespace d3server
```

Let's create the GameClient class:

```cpp
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
class Character;
}

namespace game_server {

// Forward declarations
class GameServer;
class GameSession;

/**
 * @brief Class representing a connected game client
 */
class GameClient : public std::enable_shared_from_this<GameClient> {
public:
    /**
     * @brief Constructor
     * @param socket Socket for the client connection
     * @param server Game server instance
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    GameClient(
        boost::asio::ip::tcp::socket socket,
        GameServer& server,
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~GameClient();
    
    /**
     * @brief Start processing client connection
     */
    void start();
    
    /**
     * @brief Send a message to the client
     * @param message Message to send
     */
    void sendMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Check if the client is connected
     * @return True if connected, false otherwise
     */
    bool isConnected() const;
    
    /**
     * @brief Get the client's account login
     * @return Account login
     */
    std::string getLogin() const;
    
    /**
     * @brief Set the client's account login
     * @param login Account login
     */
    void setLogin(const std::string& login);
    
    /**
     * @brief Get the client's IP address
     * @return IP address
     */
    std::string getIpAddress() const;
    
    /**
     * @brief Get the client's character ID
     * @return Character ID
     */
    uint32_t getCharacterId() const;
    
    /**
     * @brief Set the client's character
     * @param character Character
     */
    void setCharacter(const database::Character& character);
    
    /**
     * @brief Set the client's game session
     * @param session Game session
     */
    void setGameSession(std::shared_ptr<GameSession> session);
    
    /**
     * @brief Get the client's game session
     * @return Game session
     */
    std::shared_ptr<GameSession> getGameSession() const;
    
    /**
     * @brief Get the time the client connected
     * @return Connection time
     */
    std::time_t getConnectTime() const;
    
    /**
     * @brief Get the last activity time
     * @return Last activity time
     */
    std::time_t getLastActivityTime() const;
    
    /**
     * @brief Disconnect the client
     */
    void disconnect();
    
    /**
     * @brief Update the client
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);
    
private:
    /**
     * @brief Read message header from the client
     */
    void readHeader();
    
    /**
     * @brief Read message body from the client
     * @param messageSize Size of the message body
     */
    void readBody(uint32_t messageSize);
    
    /**
     * @brief Process a received message
     * @param message Message data
     */
    void processMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Write next message in the queue
     */
    void writeNextMessage();
    
    /**
     * @brief Update the last activity time
     */
    void updateLastActivityTime();
    
    /**
     * @brief Handle authentication
     * @param token Authentication token
     * @param characterId Character ID
     * @return True if authentication succeeded, false otherwise
     */
    bool handleAuth(const std::string& token, uint32_t characterId);
    
    boost::asio::ip::tcp::socket m_socket;
    GameServer& m_server;
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    std::atomic<bool> m_connected;
    std::atomic<bool> m_authenticated;
    std::string m_login;
    std::string m_ipAddress;
    uint32_t m_characterId;
    std::string m_characterName;
    int m_characterClass;
    
    std::time_t m_connectTime;
    std::time_t m_lastActivityTime;
    
    std::weak_ptr<GameSession> m_gameSession;
    
    std::vector<uint8_t> m_headerBuffer;
    std::vector<uint8_t> m_bodyBuffer;
    
    std::mutex m_writeMutex;
    std::queue<std::vector<uint8_t>> m_writeQueue;
    bool m_writing;
};

} // namespace game_server
} // namespace d3server
```

Let's implement the GameServer class:

```cpp
#include "game_server/game_server.h"
#include "game_server/game_client.h"
#include "game_server/game_session.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"

#include <chrono>
#include <algorithm>
#include <functional>

namespace d3server {
namespace game_server {

GameServer::GameServer(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_ioContext(),
    m_acceptor(m_ioContext),
    m_running(false),
    m_nextSessionId(1)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Game server instance created");
    DEBUG_FUNCTION_EXIT();
}

GameServer::~GameServer() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("Game server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool GameServer::init() {
    DEBUG_FUNCTION_ENTER();
    
    try {
        LOG_INFO("Initializing game server...");
        
        // Initialize acceptor
        auto& networkConfig = m_config->getNetworkConfig();
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(networkConfig.bindIp),
            networkConfig.gameServerPort
        );
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
        
        LOG_INFO("Game server initialized on " + networkConfig.bindIp + ":" + std::to_string(networkConfig.gameServerPort));
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during game server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void GameServer::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("Game server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    
    LOG_INFO("Starting game server...");
    
    // Start accepting connections
    startAccept();
    
    // Run the IO context in a separate thread
    m_ioThread = std::thread([this]() {
        DEBUG_CONTEXT("Game server IO thread started");
        
        try {
            m_ioContext.run();
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception in game server IO thread: " + std::string(e.what()));
        }
        
        DEBUG_LOG("Game server IO thread exited");
    });
    
    // Run the update loop in a separate thread
    m_updateThread = std::thread([this]() {
        DEBUG_CONTEXT("Game server update thread started");
        
        auto lastUpdateTime = std::chrono::steady_clock::now();
        
        while (m_running) {
            // Calculate delta time
            auto currentTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
            lastUpdateTime = currentTime;
            
            // Update sessions
            updateSessions();
            
            // Clean up disconnected clients and empty sessions
            cleanupClientsAndSessions();
            
            // Sleep to avoid high CPU usage (aim for 60 updates per second)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        DEBUG_LOG("Game server update thread exited");
    });
    
    LOG_INFO("Game server started");
    DEBUG_FUNCTION_EXIT();
}

void GameServer::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("Game server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down game server...");
    m_running = false;
    
    // Stop the IO context
    m_ioContext.stop();
    
    // Wait for threads to finish
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
    
    if (m_updateThread.joinable()) {
        m_updateThread.join();
    }
    
    // Disconnect all clients
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.clear();
    }
    
    // Clear all sessions
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        m_sessions.clear();
    }
    
    LOG_INFO("Game server shutdown complete");
    DEBUG_FUNCTION_EXIT();
}

size_t GameServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    return m_clients.size();
}

size_t GameServer::getSessionCount() const {
    std::lock_guard<std::mutex> lock(m_sessionsMutex);
    return m_sessions.size();
}

bool GameServer::isRunning() const {
    return m_running;
}

void GameServer::startAccept() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        DEBUG_LOG("Game server is not running, not accepting connections");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                // Handle the new connection
                handleAccept(std::move(socket));
            } else {
                DEBUG_LOG("Error accepting connection: " + ec.message());
            }
            
            // Accept the next connection
            startAccept();
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void GameServer::handleAccept(boost::asio::ip::tcp::socket socket) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        // Get client IP address
        std::string clientIp = socket.remote_endpoint().address().to_string();
        LOG_INFO("New game connection from " + clientIp);
        
        // Create a new client session
        auto client = std::make_shared<GameClient>(
            std::move(socket),
            *this,
            m_config,
            m_dbManager
        );
        
        // Add client to the list
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clients[client->getIpAddress()] = client;
        }
        
        // Start processing client
        client->start();
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception handling new game connection: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

void GameServer::updateSessions() {
    // Calculate delta time based on elapsed time since last update
    static auto lastUpdateTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
    lastUpdateTime = currentTime;
    
    // Update each session
    std::lock_guard<std::mutex> lock(m_sessionsMutex);
    for (auto& sessionPair : m_sessions) {
        sessionPair.second->update(deltaTime);
    }
}

void GameServer::cleanupClientsAndSessions() {
    DEBUG_FUNCTION_ENTER();
    
    // Clean up disconnected clients
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        
        std::vector<std::string> disconnectedClients;
        for (const auto& clientPair : m_clients) {
            if (!clientPair.second->isConnected()) {
                disconnectedClients.push_back(clientPair.first);
            }
        }
        
        for (const auto& ip : disconnectedClients) {
            LOG_INFO("Removing disconnected game client: " + ip);
            m_clients.erase(ip);
        }
    }
    
    // Clean up empty sessions
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        
        std::vector<uint32_t> emptySessions;
        for (const auto& sessionPair : m_sessions) {
            if (sessionPair.second->isEmpty()) {
                emptySessions.push_back(sessionPair.first);
            }
        }
        
        for (uint32_t sessionId : emptySessions) {
            LOG_INFO("Removing empty game session: " + std::to_string(sessionId));
            m_sessions.erase(sessionId);
        }
    }
    
    DEBUG_FUNCTION_EXIT();
}

std::shared_ptr<GameSession> GameServer::createGameSession() {
    DEBUG_FUNCTION_ENTER();
    
    // Generate a unique session ID
    uint32_t sessionId = m_nextSessionId++;
    
    // Create the session
    auto session = std::make_shared<GameSession>(sessionId, m_config, m_dbManager);
    
    // Initialize the session
    bool success = session->init();
    if (!success) {
        LOG_ERROR("Failed to initialize game session: " + std::to_string(sessionId));
        DEBUG_FUNCTION_EXIT();
        return nullptr;
    }
    
    // Add to the sessions map
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        m_sessions[sessionId] = session;
    }
    
    LOG_INFO("Created game session: " + std::to_string(sessionId));
    DEBUG_FUNCTION_EXIT();
    return session;
}

std::shared_ptr<GameSession> GameServer::getGameSession(uint32_t sessionId) {
    std::lock_guard<std::mutex> lock(m_sessionsMutex);
    
    auto it = m_sessions.find(sessionId);
    if (it != m_sessions.end()) {
        return it->second;
    }
    
    return nullptr;
}

} // namespace game_server
} // namespace d3server
```

Let's create documentation for the Game Server protocol:

```markdown
# D3Server Game Server Protocol Documentation

This document describes the Game Server protocol used for in-game communication between the D3Server and game clients.

## Overview

The Game Server protocol is a binary protocol designed for real-time game state updates, player actions, and world state synchronization. It uses a message-based format with a header followed by a variable-length body.

## Message Format

Each message has the following format:

```
+----------------+----------------+
| Header (4 bytes)| Body (variable) |
+----------------+----------------+
```

### Header

The header is 4 bytes long and contains the length of the body in little-endian format.

```
+-------------------+-------------------+-------------------+-------------------+
| Length (byte 0)   | Length (byte 1)   | Length (byte 2)   | Length (byte 3)   |
+-------------------+-------------------+-------------------+-------------------+
```

### Body

The body contains the message data, with the first byte indicating the message type.

```
+-------------------+-------------------+
| Message Type      | Message Data      |
+-------------------+-------------------+
```

## Message Types

### Server to Client

| Type (Hex) | Description               | Format                                                |
|------------|---------------------------|-------------------------------------------------------|
| 0x01       | Welcome                   | [0x01] [Version (4 bytes)]                           |
| 0x02       | Authentication Result     | [0x02] [Result (1 = Success, 0 = Fail)]              |
| 0x04       | Pong                      | [0x04]                                               |
| 0x06       | Enter Session Confirmation| [0x06] [Session ID (4 bytes)]                        |
| 0x07       | Player Joined             | [0x07] [Player Data]                                 |
| 0x08       | Player Left               | [0x08] [Player ID (4 bytes)]                         |
| 0x0A       | World State               | [0x0A] [World Data]                                  |
| 0x0C       | Entity Update             | [0x0C] [Entity ID (4 bytes)] [Entity Data]           |
| 0x0E       | Chat Message              | [0x0E] [Sender ID (4 bytes)] [Msg Len] [Message]     |
| 0x10       | Game Event                | [0x10] [Event Type (1 byte)] [Event Data]            |
| 0x12       | Item Data                 | [0x12] [Item ID (4 bytes)] [Item Data]               |
| 0x14       | Skill Effect              | [0x14] [Caster ID (4 bytes)] [Skill ID] [Effect Data]|

### Client to Server

| Type (Hex) | Description               | Format                                                   |
|------------|---------------------------|----------------------------------------------------------|
| 0x01       | Authentication Request    | [0x01] [Token Len (1 byte)] [Token] [Character ID (4 bytes)] |
| 0x03       | Ping                      | [0x03]                                                  |
| 0x05       | Enter Session Request     | [0x05] [Session ID (4 bytes) or 0 for new]              |
| 0x09       | Player Action             | [0x09] [Action Type (1 byte)] [Action Data]             |
| 0x0B       | Player Position           | [0x0B] [Position X (4 bytes)] [Position Y (4 bytes)] [Position Z (4 bytes)] |
| 0x0D       | Chat Message              | [0x0D] [Target Type (1 byte)] [Target ID (4 bytes)] [Msg Len] [Message] |
| 0x0F       | Skill Use                 | [0x0F] [Skill ID (2 bytes)] [Target Type] [Target Data] |
| 0x11       | Item Interaction          | [0x11] [Interaction Type] [Item ID (4 bytes)]           |
| 0x13       | Command                   | [0x13] [Command Len] [Command]                          |

## Player Data Format

The player data format used in Player Joined messages:

```
+----------------+----------------+------------------+----------------+------------------+
| Player ID      | Name Length    | Player Name      | Class ID       | Level            |
+----------------+----------------+------------------+----------------+------------------+
| Position X     | Position Y     | Position Z       | Rotation       | Visual Equipment |
+----------------+----------------+------------------+----------------+------------------+
```

## Action Types

| ID  | Action Name            |
|-----|------------------------|
| 0x01| Move                   |
| 0x02| Attack                 |
| 0x03| Use Skill              |
| 0x04| Use Item               |
| 0x05| Pick Up Item           |
| 0x06| Interact with Object   |
| 0x07| Cast Town Portal       |
| 0x08| Use Potion             |
| 0x09| Revive                 |

## Position Updates

Position updates (Type 0x0B) are frequent messages that update the server about the client's current position. For optimization, these messages use a compressed format with smaller floating-point representations.

## Entity Types

| ID  | Entity Type            |
|-----|------------------------|
| 0x01| Player                 |
| 0x02| NPC                    |
| 0x03| Monster                |
| 0x04| Item                   |
| 0x05| Projectile             |
| 0x06| Effect                 |
| 0x07| Interaction Object     |
| 0x08| Portal                 |

## World Data Format

The world data format provides information about the current game world:

```
+----------------+----------------+------------------+-------------------------+
| World ID       | Difficulty     | Game Mode        | Number of Entities      |
+----------------+----------------+------------------+-------------------------+
| Entity Array   | Item Array     | Object Array     | Terrain Seed            |
+----------------+----------------+------------------+-------------------------+
```

## Authentication Process

1. Server sends Welcome message (0x01) upon client connection.
2. Client sends Authentication Request (0x01) with a token from Battle.net server.
3. Server validates token and sends Authentication Result (0x02).
4. If authentication is successful, client can request to enter a session.

## Session Handling

1. Client sends Enter Session Request (0x05) with session ID (or 0 for a new session).
2. Server creates or joins session and sends Enter Session Confirmation (0x06).
3. Server sends Player Joined (0x07) messages for all existing players in the session.
4. Server sends World State (0x0A) with the current game state.

## Example Message Flow

### Authentication

1. Server → Client: Welcome
   ```
   Header: 05 00 00 00
   Body:   01 01 00 00 00
   ```

2. Client → Server: Authentication Request
   ```
   Header: 15 00 00 00
   Body:   01 10 "auth_token_value" 01 00 00 00
   ```

3. Server → Client: Authentication Result (Success)
   ```
   Header: 02 00 00 00
   Body:   02 01
   ```

### Enter Game Session

1. Client → Server: Enter Session Request (New Session)
   ```
   Header: 05 00 00 00
   Body:   05 00 00 00 00
   ```

2. Server → Client: Enter Session Confirmation
   ```
   Header: 05 00 00 00
   Body:   06 01 00 00 00
   ```

### Player Movement

1. Client → Server: Player Position
   ```
   Header: 0D 00 00 00
   Body:   0B 00 00 80 41 00 00 00 00 00 00 80 41
   ```

2. Server → Client: Entity Update (for other players)
   ```
   Header: 0D 00 00 00
   Body:   0C 02 00 00 00 00 00 80 41 00 00 00 00 00 00 80 41
   ```

## Implementation Notes

- All multi-byte integer values are transmitted in little-endian format.
- Floating-point values are transmitted in IEEE 754 format.
- Strings are transmitted with a length prefix followed by the raw string bytes (no null terminator).
- Position updates use a smaller format for bandwidth optimization.
- The server enforces a maximum message size (currently 1MB) to prevent memory exhaustion attacks.
- For better performance, position updates may be batched or sent at a lower frequency for distant players.
- The protocol uses binary format rather than text-based formats for better bandwidth efficiency.
- Delta compression may be applied to world state updates to reduce bandwidth usage.
- The protocol supports incremental updates to avoid sending complete world state for every change.
```

I'll continue assisting with the Diablo 3 server implementation in C++. Let me know what you'd like to focus on next.

---

**User**

请完善下 tests里的内容，另外 docs里面既有中文又需要有英文，请给出一份中文的文档放到docs里面，需要包含我们的实现细节，注意事项。

---

**Cursor**

I'll help you create a comprehensive Chinese documentation and add tests for the D3Server project. Let's start by examining the current project structure to better understand what needs to be implemented.

Let's check if the tests directory exists and create it if needed. I'll also look at the overall project structure:

Let's check the test directory:

Let's examine the unit directory to see what tests we have so far:

Let's check the existing test CMakeLists.txt to understand the testing framework:

Now let's check the docs directory:

Let's first create the Chinese documentation for our implementation. I'll create a comprehensive implementation guide in Chinese that covers all aspects of our Diablo 3 server implementation.

```markdown
# D3Server 实现指南

## 项目概述

D3Server 是一个使用 C++ 实现的 Diablo 3 服务器，旨在提供一个完整的游戏服务器解决方案，包含认证、角色管理、游戏会话和管理功能。项目采用模块化设计，使用现代 C++ 特性和 Boost 库，遵循清晰的架构设计原则。

## 系统架构

### 核心模块组成

1. **核心组件 (core)**
   - `Server` 类：协调所有服务器组件
   - `Config` 类：处理服务器配置，使用 SQLite 存储
   - `Logger` 类：提供详细的日志记录，支持调试模式

2. **网络模块**
   - **REST API**：使用 Boost.Asio 实现的 HTTP 服务器，提供账户和管理操作接口
   - **Battle.net 服务器**：处理认证和游戏协调
   - **游戏服务器**：管理游戏会话和实时游戏玩法

3. **数据库模块**
   - 使用 SQLite 进行数据存储
   - 提供账号和角色数据访问对象 (DAO)
   - 支持事务处理和并发访问

## 关键功能实现

### 认证系统

认证系统通过 Battle.net 服务器实现，步骤如下：

1. 客户端连接到 Battle.net 服务器
2. 服务器发送欢迎消息
3. 客户端发送登录和密码
4. 服务器验证凭据并返回认证结果
5. 认证成功后，客户端可以请求角色列表或创建角色

密码存储使用安全哈希算法，避免明文存储。认证令牌用于后续游戏会话验证。

### 游戏会话管理

游戏会话管理由游戏服务器处理：

1. 客户端使用 Battle.net 认证令牌连接到游戏服务器
2. 服务器验证令牌并允许客户端加入游戏会话
3. 服务器广播玩家加入消息给会话中的其他玩家
4. 服务器发送当前世界状态给新加入的玩家
5. 会话中实时同步玩家位置、动作和游戏事件

游戏会话支持不同的难度级别和游戏模式，玩家可以选择加入现有会话或创建新会话。

### REST API

REST API 提供以下功能：

1. **账户管理**：创建、读取、更新和删除账户
2. **角色管理**：创建和查询角色
3. **管理功能**：封禁账户、查询服务器状态、获取日志
4. **服务器控制**：关闭和重启服务器

API 使用 JSON 格式进行数据交换，并使用 Bearer 令牌进行认证。详细的 API 端点和参数可在 `rest_api.md` 文件中查看。

## 网络协议

### Battle.net 协议

Battle.net 协议是一个二进制协议，用于认证和游戏协调。每个消息包含一个 4 字节的头部（指定消息长度）和一个变长的消息体。详细的消息类型和格式可在 `battle_net_protocol.md` 文件中查看。

### 游戏服务器协议

游戏服务器协议是一个专为实时游戏状态更新设计的二进制协议。它包含各种消息类型，用于同步玩家位置、动作、实体更新和游戏事件。详细的消息类型和格式可在 `game_server_protocol.md` 文件中查看。

## 编译和部署

### 系统要求

- 现代 C++ 编译器（支持 C++17）
- Boost 库（1.70 或更高版本）
- CMake（3.10 或更高版本）
- SQLite3
- 用于测试的 Google Test 框架

### 编译步骤

```bash
mkdir build
cd build
cmake ..
make
```

### 配置

服务器配置存储在 SQLite 数据库中，包括：

- 网络设置（IP、端口）
- 数据库连接配置
- 日志级别和路径
- 游戏规则参数（最大角色数、最大玩家数等）

可以通过 REST API 或配置文件修改这些设置。

## 调试功能

### 调试模式

启用调试模式后，服务器会记录详细的调试信息，包括：

- 函数进入和退出跟踪
- 变量值和状态变化
- 性能计时

调试日志格式包含时间戳、线程 ID、源文件和行号，便于问题定位。

### 性能监控

系统包含性能监控功能，可以跟踪：

- CPU 和内存使用率
- 网络流量和延迟
- 数据库查询性能
- 游戏会话和玩家统计

## 注意事项和最佳实践

### 安全性考虑

1. **输入验证**：所有客户端输入都经过严格验证，防止注入攻击
2. **授权检查**：所有 API 端点实施严格的授权检查
3. **密码处理**：密码经过安全哈希存储，不会以明文形式出现
4. **请求限流**：防止暴力攻击的请求限制机制
5. **网络安全**：关键数据应考虑加密传输

### 性能优化

1. **连接池**：数据库连接池优化数据库访问
2. **异步处理**：使用异步 IO 提高并发性能
3. **消息批处理**：游戏服务器合并小消息减少网络开销
4. **差异更新**：仅发送世界状态的变化部分
5. **资源管理**：注意内存和文件描述符管理

### 错误处理

1. **异常捕获**：所有外部操作应在 try-catch 块中执行
2. **优雅降级**：在组件失败时提供备用功能
3. **日志记录**：详细记录错误原因和上下文
4. **重试机制**：对暂时性故障实施智能重试策略

## 测试策略

项目使用 Google Test 框架进行单元测试和集成测试：

1. **单元测试**：测试各个组件的独立功能
2. **集成测试**：测试组件之间的交互
3. **负载测试**：验证系统在高负载下的性能
4. **安全测试**：检查常见安全漏洞

测试覆盖率目标为 80% 或更高。

## 扩展和未来改进

1. **更多游戏功能**：物品系统、技能系统、怪物 AI
2. **群组服务器**：支持多服务器集群部署
3. **WebSocket 支持**：为 Web 客户端提供支持
4. **更多安全功能**：强双因素认证、OAuth 集成
5. **自动化部署**：Docker 容器和持续集成

## 常见问题和解决方案

1. **连接问题**：检查防火墙设置和端口配置
2. **性能下降**：分析日志查找瓶颈，检查数据库索引
3. **内存泄漏**：使用 Valgrind 等工具检测和修复
4. **崩溃问题**：核心转储分析和调试日志检查
5. **并发问题**：使用线程分析工具检测死锁和竞争条件

## 项目维护

1. **代码风格**：遵循一致的代码风格和命名约定
2. **文档更新**：代码更改时同步更新文档
3. **版本控制**：使用语义化版本控制和清晰的提交信息
4. **代码审查**：实施严格的代码审查流程
5. **技术债务**：定期重构和清理技术债务

## 联系和支持

如有问题或需要支持，请联系项目维护人员：

- 邮箱：support@d3server.example.com
- 问题追踪：https://github.com/example/d3server/issues

---

**版权声明**：本文档仅供项目开发者使用，未经许可不得外传。
```

Now, let's implement the tests. Let's start by creating the test files according to the CMakeLists.txt structure. First, let's create the core tests:

```cpp
#include <gtest/gtest.h>
#include "core/logger.h"
#include "utils/debug.h"

int main(int argc, char** argv) {
    // Initialize the logger for tests
    d3server::core::Logger::getInstance().init("d3server_test.log", true);
    
    // Turn off debug output during tests by default
    d3server::utils::Debug::setEnabled(false);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    return result;
}
```

Next, let's implement the Config tests to ensure the configuration system is working properly:

```cpp
#include <gtest/gtest.h>
#include "core/config.h"
#include <memory>
#include <string>
#include <filesystem>

namespace d3server {
namespace core {
namespace test {

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test config database in memory
        m_config = std::make_shared<Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
    }
    
    void TearDown() override {
        m_config.reset();
    }
    
    std::shared_ptr<Config> m_config;
};

// Test server configuration
TEST_F(ConfigTest, ServerConfig) {
    // Get server config
    auto serverConfig = m_config->getServerConfig();
    
    // Test default values
    EXPECT_FALSE(serverConfig.serverName.empty());
    EXPECT_GE(serverConfig.maxPlayers, 1);
    EXPECT_GE(serverConfig.maxCharactersPerAccount, 1);
    
    // Test setting and getting values
    serverConfig.serverName = "TestServer";
    serverConfig.maxPlayers = 100;
    serverConfig.maxCharactersPerAccount = 10;
    serverConfig.enableDebug = true;
    
    // Update config
    EXPECT_TRUE(m_config->updateServerConfig(serverConfig));
    
    // Get updated config
    auto updatedConfig = m_config->getServerConfig();
    
    // Verify values were saved
    EXPECT_EQ(updatedConfig.serverName, "TestServer");
    EXPECT_EQ(updatedConfig.maxPlayers, 100);
    EXPECT_EQ(updatedConfig.maxCharactersPerAccount, 10);
    EXPECT_TRUE(updatedConfig.enableDebug);
}

// Test network configuration
TEST_F(ConfigTest, NetworkConfig) {
    // Get network config
    auto networkConfig = m_config->getNetworkConfig();
    
    // Test default values
    EXPECT_FALSE(networkConfig.bindIp.empty());
    EXPECT_GT(networkConfig.restApiPort, 0);
    EXPECT_GT(networkConfig.battleNetPort, 0);
    EXPECT_GT(networkConfig.gameServerPort, 0);
    
    // Test setting and getting values
    networkConfig.bindIp = "127.0.0.1";
    networkConfig.restApiPort = 8080;
    networkConfig.battleNetPort = 1119;
    networkConfig.gameServerPort = 1120;
    
    // Update config
    EXPECT_TRUE(m_config->updateNetworkConfig(networkConfig));
    
    // Get updated config
    auto updatedConfig = m_config->getNetworkConfig();
    
    // Verify values were saved
    EXPECT_EQ(updatedConfig.bindIp, "127.0.0.1");
    EXPECT_EQ(updatedConfig.restApiPort, 8080);
    EXPECT_EQ(updatedConfig.battleNetPort, 1119);
    EXPECT_EQ(updatedConfig.gameServerPort, 1120);
}

// Test database configuration
TEST_F(ConfigTest, DatabaseConfig) {
    // Get database config
    auto dbConfig = m_config->getDatabaseConfig();
    
    // Test default values
    EXPECT_FALSE(dbConfig.dbPath.empty());
    
    // Test setting and getting values
    dbConfig.dbPath = "test_db.sqlite";
    
    // Update config
    EXPECT_TRUE(m_config->updateDatabaseConfig(dbConfig));
    
    // Get updated config
    auto updatedConfig = m_config->getDatabaseConfig();
    
    // Verify values were saved
    EXPECT_EQ(updatedConfig.dbPath, "test_db.sqlite");
}

// Test config file loading and saving
TEST_F(ConfigTest, ConfigFileSaveLoad) {
    // Create temporary file path
    std::string tempConfigPath = "temp_config_test.db";
    
    // Create new config instance
    auto configToSave = std::make_shared<Config>();
    ASSERT_TRUE(configToSave->init(tempConfigPath));
    
    // Set some values
    auto serverConfig = configToSave->getServerConfig();
    serverConfig.serverName = "SaveLoadTest";
    serverConfig.maxPlayers = 200;
    EXPECT_TRUE(configToSave->updateServerConfig(serverConfig));
    
    // Close the config
    configToSave.reset();
    
    // Load the config back
    auto loadedConfig = std::make_shared<Config>();
    ASSERT_TRUE(loadedConfig->init(tempConfigPath));
    
    // Verify values were loaded
    auto loadedServerConfig = loadedConfig->getServerConfig();
    EXPECT_EQ(loadedServerConfig.serverName, "SaveLoadTest");
    EXPECT_EQ(loadedServerConfig.maxPlayers, 200);
    
    // Clean up
    loadedConfig.reset();
    std::filesystem::remove(tempConfigPath);
}

} // namespace test
} // namespace core
} // namespace d3server
```

Now, let's implement tests for the logger class:

```cpp
#include <gtest/gtest.h>
#include "core/logger.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <regex>

namespace d3server {
namespace core {
namespace test {

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test log file path
        m_logFilePath = "test_logger.log";
        
        // Initialize logger with test file
        Logger::getInstance().init(m_logFilePath, true);
    }
    
    void TearDown() override {
        // Reset logger
        Logger::getInstance().shutdown();
        
        // Clean up test log file
        if (std::filesystem::exists(m_logFilePath)) {
            std::filesystem::remove(m_logFilePath);
        }
    }
    
    // Helper to check if log file contains a message
    bool logContainsMessage(const std::string& message) {
        if (!std::filesystem::exists(m_logFilePath)) {
            return false;
        }
        
        std::ifstream logFile(m_logFilePath);
        std::string content((std::istreambuf_iterator<char>(logFile)),
                           std::istreambuf_iterator<char>());
        
        return content.find(message) != std::string::npos;
    }
    
    // Helper to check if log entry has the correct format
    bool logHasCorrectFormat(const std::string& level) {
        if (!std::filesystem::exists(m_logFilePath)) {
            return false;
        }
        
        std::ifstream logFile(m_logFilePath);
        std::string line;
        
        // Find a line with the specified level
        while (std::getline(logFile, line)) {
            if (line.find("[" + level + "]") != std::string::npos) {
                // Regular expression for log format: [LEVEL] YYYY-MM-DD HH:MM:SS - Message
                std::regex logPattern("\\[" + level + "\\] \\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2} - .*");
                return std::regex_match(line, logPattern);
            }
        }
        
        return false;
    }
    
    std::string m_logFilePath;
};

// Test basic logging functionality
TEST_F(LoggerTest, BasicLogging) {
    // Log messages with different levels
    LOG_DEBUG("Debug test message");
    LOG_INFO("Info test message");
    LOG_WARNING("Warning test message");
    LOG_ERROR("Error test message");
    
    // Verify messages are in the log file
    EXPECT_TRUE(logContainsMessage("Debug test message"));
    EXPECT_TRUE(logContainsMessage("Info test message"));
    EXPECT_TRUE(logContainsMessage("Warning test message"));
    EXPECT_TRUE(logContainsMessage("Error test message"));
}

// Test log format
TEST_F(LoggerTest, LogFormat) {
    // Log messages
    LOG_INFO("Info test message for format check");
    LOG_ERROR("Error test message for format check");
    
    // Check format for different levels
    EXPECT_TRUE(logHasCorrectFormat("INFO"));
    EXPECT_TRUE(logHasCorrectFormat("ERROR"));
}

// Test log level filtering
TEST_F(LoggerTest, LogLevelFiltering) {
    // Create a new logger with INFO level
    std::string filteredLogFile = "filtered_test.log";
    Logger testLogger;
    testLogger.init(filteredLogFile, false, LogLevel::INFO);
    
    // Log messages
    testLogger.debug("Debug message that should be filtered");
    testLogger.info("Info message that should be logged");
    testLogger.warning("Warning message that should be logged");
    testLogger.error("Error message that should be logged");
    
    // Read log file
    std::ifstream logFile(filteredLogFile);
    std::string content((std::istreambuf_iterator<char>(logFile)),
                       std::istreambuf_iterator<char>());
    
    // Verify filtering
    EXPECT_EQ(content.find("Debug message that should be filtered"), std::string::npos);
    EXPECT_NE(content.find("Info message that should be logged"), std::string::npos);
    EXPECT_NE(content.find("Warning message that should be logged"), std::string::npos);
    EXPECT_NE(content.find("Error message that should be logged"), std::string::npos);
    
    // Clean up
    testLogger.shutdown();
    if (std::filesystem::exists(filteredLogFile)) {
        std::filesystem::remove(filteredLogFile);
    }
}

// Test concurrent logging
TEST_F(LoggerTest, ConcurrentLogging) {
    const int numThreads = 5;
    const int messagesPerThread = 10;
    
    // Create threads that write log messages concurrently
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++) {
        threads.push_back(std::thread([i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; j++) {
                LOG_INFO("Concurrent log message from thread " + std::to_string(i) + ", message " + std::to_string(j));
            }
        }));
    }
    
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Count the number of log messages
    int messageCount = 0;
    std::ifstream logFile(m_logFilePath);
    std::string line;
    while (std::getline(logFile, line)) {
        if (line.find("Concurrent log message from thread") != std::string::npos) {
            messageCount++;
        }
    }
    
    // Verify all messages were logged
    EXPECT_EQ(messageCount, numThreads * messagesPerThread);
}

} // namespace test
} // namespace core
} // namespace d3server
```

Now, let's implement tests for the REST API's account controller:

```cpp
#include <gtest/gtest.h>
#include "core/logger.h"
#include "utils/debug.h"

int main(int argc, char** argv) {
    // Initialize the logger for tests
    d3server::core::Logger::getInstance().init("rest_api_test.log", true);
    
    // Turn off debug output during tests by default
    d3server::utils::Debug::setEnabled(false);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    return result;
}
```

```cpp
#include <gtest/gtest.h>
#include "rest_api/account_controller.h"
#include "core/config.h"
#include "database/database_manager.h"
#include <memory>
#include <string>

namespace d3server {
namespace rest_api {
namespace test {

class AccountControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Create account controller
        m_accountController = std::make_shared<AccountController>(m_config, m_dbManager);
    }
    
    void TearDown() override {
        m_accountController.reset();
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Helper to create a test account
    bool createTestAccount(const std::string& login, const std::string& email, const std::string& password) {
        HttpRequest request;
        request.method = "POST";
        request.uri = "/api/accounts";
        request.headers["Content-Type"] = "application/json";
        request.body = "{\"login\":\"" + login + "\",\"email\":\"" + email + "\",\"password\":\"" + password + "\"}";
        
        HttpResponse response = m_accountController->createAccount(request);
        return response.statusCode == 201;
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<AccountController> m_accountController;
};

// Test account creation
TEST_F(AccountControllerTest, CreateAccount) {
    // Create valid request
    HttpRequest request;
    request.method = "POST";
    request.uri = "/api/accounts";
    request.headers["Content-Type"] = "application/json";
    request.body = R"({"login":"testuser","email":"test@example.com","password":"password123"})";
    
    // Call controller
    HttpResponse response = m_accountController->createAccount(request);
    
    // Verify success
    EXPECT_EQ(response.statusCode, 201);
    EXPECT_EQ(response.statusMessage, "Created");
    EXPECT_EQ(response.headers["Content-Type"], "application/json");
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Try creating duplicate account
    response = m_accountController->createAccount(request);
    
    // Verify conflict
    EXPECT_EQ(response.statusCode, 409);
    EXPECT_NE(response.body.find("already exists"), std::string::npos);
}

// Test invalid account creation
TEST_F(AccountControllerTest, CreateAccountInvalid) {
    // Test missing login
    HttpRequest request;
    request.method = "POST";
    request.uri = "/api/accounts";
    request.headers["Content-Type"] = "application/json";
    request.body = R"({"email":"test@example.com","password":"password123"})";
    
    HttpResponse response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
    
    // Test invalid email
    request.body = R"({"login":"testuser2","email":"invalid-email","password":"password123"})";
    response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
    
    // Test short password
    request.body = R"({"login":"testuser3","email":"test@example.com","password":"short"})";
    response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
    
    // Test invalid JSON
    request.body = R"({"login":"testuser4","email":"test@example.com","password":})";
    response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
}

// Test getting account
TEST_F(AccountControllerTest, GetAccount) {
    // Create test account
    ASSERT_TRUE(createTestAccount("getuser", "get@example.com", "password123"));
    
    // Create request to get account
    HttpRequest request;
    request.method = "GET";
    request.uri = "/api/accounts/getuser";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    
    // Call controller
    HttpResponse response = m_accountController->getAccount(request, "getuser");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_EQ(response.statusMessage, "OK");
    EXPECT_EQ(response.headers["Content-Type"], "application/json");
    EXPECT_NE(response.body.find("getuser"), std::string::npos);
    EXPECT_NE(response.body.find("get@example.com"), std::string::npos);
    
    // Test non-existent account
    response = m_accountController->getAccount(request, "nonexistent");
    EXPECT_EQ(response.statusCode, 404);
}

// Test updating account
TEST_F(AccountControllerTest, UpdateAccount) {
    // Create test account
    ASSERT_TRUE(createTestAccount("updateuser", "update@example.com", "password123"));
    
    // Create request to update account
    HttpRequest request;
    request.method = "PUT";
    request.uri = "/api/accounts/updateuser";
    request.headers["Content-Type"] = "application/json";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    request.body = R"({"email":"updated@example.com","password":"newpassword123"})";
    
    // Call controller
    HttpResponse response = m_accountController->updateAccount(request, "updateuser");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Get updated account
    request.method = "GET";
    request.uri = "/api/accounts/updateuser";
    response = m_accountController->getAccount(request, "updateuser");
    
    // Verify updated email
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("updated@example.com"), std::string::npos);
}

// Test deleting account
TEST_F(AccountControllerTest, DeleteAccount) {
    // Create test account
    ASSERT_TRUE(createTestAccount("deleteuser", "delete@example.com", "password123"));
    
    // Create request to delete account
    HttpRequest request;
    request.method = "DELETE";
    request.uri = "/api/accounts/deleteuser";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    
    // Call controller
    HttpResponse response = m_accountController->deleteAccount(request, "deleteuser");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Try to get deleted account
    request.method = "GET";
    request.uri = "/api/accounts/deleteuser";
    response = m_accountController->getAccount(request, "deleteuser");
    
    // Verify account not found
    EXPECT_EQ(response.statusCode, 404);
}

// Test getting characters
TEST_F(AccountControllerTest, GetCharacters) {
    // Create test account
    ASSERT_TRUE(createTestAccount("charuser", "char@example.com", "password123"));
    
    // Create request to get characters
    HttpRequest request;
    request.method = "GET";
    request.uri = "/api/accounts/charuser/characters";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    
    // Call controller
    HttpResponse response = m_accountController->getCharacters(request, "charuser");
    
    // Verify success (empty list)
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_EQ(response.headers["Content-Type"], "application/json");
    EXPECT_NE(response.body.find("[]"), std::string::npos);
}

// Test creating character
TEST_F(AccountControllerTest, CreateCharacter) {
    // Create test account
    ASSERT_TRUE(createTestAccount("charuser2", "char2@example.com", "password123"));
    
    // Create request to create character
    HttpRequest request;
    request.method = "POST";
    request.uri = "/api/accounts/charuser2/characters";
    request.headers["Content-Type"] = "application/json";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    request.body = R"({"name":"TestChar","class_id":1})";
    
    // Call controller
    HttpResponse response = m_accountController->createCharacter(request, "charuser2");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 201);
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Get characters
    request.method = "GET";
    request.uri = "/api/accounts/charuser2/characters";
    response = m_accountController->getCharacters(request, "charuser2");
    
    // Verify character was created
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("TestChar"), std::string::npos);
    EXPECT_NE(response.body.find("\"class_id\":1"), std::string::npos);
}

} // namespace test
} // namespace rest_api
} // namespace d3server
```

Now, let's create tests for the Authentication module in the Battle.net server:

```cpp
#include <gtest/gtest.h>
#include "core/logger.h"
#include "utils/debug.h"

int main(int argc, char** argv) {
    // Initialize the logger for tests
    d3server::core::Logger::getInstance().init("battle_net_test.log", true);
    
    // Turn off debug output during tests by default
    d3server::utils::Debug::setEnabled(false);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    return result;
}
```

```cpp
#include <gtest/gtest.h>
#include "battle_net/battle_net_client.h"
#include "battle_net/battle_net_server.h"
#include "core/config.h"
#include "database/database_manager.h"
#include "utils/crypto_utils.h"
#include <memory>
#include <vector>
#include <string>

namespace d3server {
namespace battle_net {
namespace test {

class AuthTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Setup test database
        setupTestDatabase();
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Setup test database with test accounts
    void setupTestDatabase() {
        // Create test accounts
        ASSERT_TRUE(m_dbManager->createAccount("testuser", "test@example.com", "password123"));
        ASSERT_TRUE(m_dbManager->createAccount("banneduser", "banned@example.com", "password123"));
        
        // Ban one of the accounts
        ASSERT_TRUE(m_dbManager->setBanStatus("banneduser", true, "Test ban reason"));
    }
    
    // Helper to create an authentication message
    std::vector<uint8_t> createAuthMessage(const std::string& login, const std::string& password) {
        std::vector<uint8_t> message;
        
        // Message type: Auth (0x01)
        message.push_back(0x01);
        
        // Login length
        message.push_back(static_cast<uint8_t>(login.length()));
        
        // Login
        message.insert(message.end(), login.begin(), login.end());
        
        // Password length
        message.push_back(static_cast<uint8_t>(password.length()));
        
        // Password
        message.insert(message.end(), password.begin(), password.end());
        
        return message;
    }
    
    // Mock client for testing
    class MockBattleNetClient : public BattleNetClient {
    public:
        MockBattleNetClient(BattleNetServer& server, 
                            std::shared_ptr<core::Config> config,
                            std::shared_ptr<database::DatabaseManager> dbManager)
            : BattleNetClient(boost::asio::ip::tcp::socket(m_ioContext), server, config, dbManager),
              m_lastSentMessage(0) {
        }
        
        // Override to capture sent messages
        void sendMessage(const std::vector<uint8_t>& message) override {
            m_lastSentMessage = message;
        }
        
        // Process a message directly
        bool processTestMessage(const std::vector<uint8_t>& message) {
            try {
                processMessage(message);
                return true;
            }
            catch (...) {
                return false;
            }
        }
        
        // Get the last sent message
        std::vector<uint8_t> getLastSentMessage() const {
            return m_lastSentMessage;
        }
        
    private:
        boost::asio::io_context m_ioContext;
        std::vector<uint8_t> m_lastSentMessage;
    };
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
};

// Test successful authentication
TEST_F(AuthTest, SuccessfulAuth) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message
    auto authMessage = createAuthMessage("testuser", "password123");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::Authenticated);
    EXPECT_EQ(client.getLogin(), "testuser");
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x01); // Success
}

// Test failed authentication with wrong password
TEST_F(AuthTest, FailedAuthWrongPassword) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message with wrong password
    auto authMessage = createAuthMessage("testuser", "wrongpassword");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::AuthFailed);
    EXPECT_TRUE(client.getLogin().empty());
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x00); // Failure
}

// Test failed authentication with non-existent user
TEST_F(AuthTest, FailedAuthNonExistentUser) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message with non-existent user
    auto authMessage = createAuthMessage("nonexistent", "password123");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::AuthFailed);
    EXPECT_TRUE(client.getLogin().empty());
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x00); // Failure
}

// Test banned account authentication
TEST_F(AuthTest, FailedAuthBannedAccount) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message for banned account
    auto authMessage = createAuthMessage("banneduser", "password123");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::AuthFailed);
    EXPECT_TRUE(client.getLogin().empty());
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x00); // Failure
}

// Test malformed authentication message
TEST_F(AuthTest, MalformedAuthMessage) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create malformed auth message (too short)
    std::vector<uint8_t> malformedMessage = {0x01, 0x08};
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(malformedMessage));
    
    // Check authentication state - should still be not authenticated
    EXPECT_EQ(client.getAuthState(), AuthState::NotAuthenticated);
}

// Test ping-pong messages
TEST_F(AuthTest, PingPong) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create ping message
    std::vector<uint8_t> pingMessage = {0x03};
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(pingMessage));
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x04); // Pong message type
}

} // namespace test
} // namespace battle_net
} // namespace d3server
```

Finally, let's implement some tests for the Game Server's player functionality:

```cpp
#include <gtest/gtest.h>
#include "core/logger.h"
#include "utils/debug.h"

int main(int argc, char** argv) {
    // Initialize the logger for tests
    d3server::core::Logger::getInstance().init("game_server_test.log", true);
    
    // Turn off debug output during tests by default
    d3server::utils::Debug::setEnabled(false);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    return result;
}
```

```cpp
#include <gtest/gtest.h>
#include "game_server/game_client.h"
#include "game_server/game_server.h"
#include "game_server/game_session.h"
#include "core/config.h"
#include "database/database_manager.h"
#include <memory>
#include <vector>
#include <string>

namespace d3server {
namespace game_server {
namespace test {

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Setup test database
        setupTestDatabase();
        
        // Create game server
        m_gameServer = std::make_shared<GameServer>(m_config, m_dbManager);
        ASSERT_TRUE(m_gameServer->init());
    }
    
    void TearDown() override {
        m_gameServer.reset();
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Setup test database with test accounts and characters
    void setupTestDatabase() {
        // Create test accounts
        ASSERT_TRUE(m_dbManager->createAccount("player1", "player1@example.com", "password123"));
        ASSERT_TRUE(m_dbManager->createAccount("player2", "player2@example.com", "password123"));
        
        // Create characters
        database::Character char1;
        char1.name = "Barbarian1";
        char1.classId = 1; // Barbarian
        char1.level = 10;
        ASSERT_TRUE(m_dbManager->createCharacter("player1", char1));
        
        database::Character char2;
        char2.name = "Wizard1";
        char2.classId = 3; // Wizard
        char2.level = 15;
        ASSERT_TRUE(m_dbManager->createCharacter("player2", char2));
    }
    
    // Helper to create an authentication message
    std::vector<uint8_t> createAuthMessage(const std::string& token, uint32_t characterId) {
        std::vector<uint8_t> message;
        
        // Message type: Auth (0x01)
        message.push_back(0x01);
        
        // Token length
        message.push_back(static_cast<uint8_t>(token.length()));
        
        // Token
        message.insert(message.end(), token.begin(), token.end());
        
        // Character ID (4 bytes, little-endian)
        message.push_back(characterId & 0xFF);
        message.push_back((characterId >> 8) & 0xFF);
        message.push_back((characterId >> 16) & 0xFF);
        message.push_back((characterId >> 24) & 0xFF);
        
        return message;
    }
    
    // Helper to create a session join message
    std::vector<uint8_t> createJoinSessionMessage(uint32_t sessionId) {
        std::vector<uint8_t> message;
        
        // Message type: Enter Session Request (0x05)
        message.push_back(0x05);
        
        // Session ID (4 bytes, little-endian)
        message.push_back(sessionId & 0xFF);
        message.push_back((sessionId >> 8) & 0xFF);
        message.push_back((sessionId >> 16) & 0xFF);
        message.push_back((sessionId >> 24) & 0xFF);
        
        return message;
    }
    
    // Helper to create a player position message
    std::vector<uint8_t> createPositionMessage(float x, float y, float z) {
        std::vector<uint8_t> message;
        
        // Message type: Player Position (0x0B)
        message.push_back(0x0B);
        
        // Position X (4 bytes, IEEE 754 float)
        uint32_t xBits;
        memcpy(&xBits, &x, sizeof(float));
        message.push_back(xBits & 0xFF);
        message.push_back((xBits >> 8) & 0xFF);
        message.push_back((xBits >> 16) & 0xFF);
        message.push_back((xBits >> 24) & 0xFF);
        
        // Position Y (4 bytes, IEEE 754 float)
        uint32_t yBits;
        memcpy(&yBits, &y, sizeof(float));
        message.push_back(yBits & 0xFF);
        message.push_back((yBits >> 8) & 0xFF);
        message.push_back((yBits >> 16) & 0xFF);
        message.push_back((yBits >> 24) & 0xFF);
        
        // Position Z (4 bytes, IEEE 754 float)
        uint32_t zBits;
        memcpy(&zBits, &z, sizeof(float));
        message.push_back(zBits & 0xFF);
        message.push_back((zBits >> 8) & 0xFF);
        message.push_back((zBits >> 16) & 0xFF);
        message.push_back((zBits >> 24) & 0xFF);
        
        return message;
    }
    
    // Helper to create a skill use message
    std::vector<uint8_t> createSkillUseMessage(uint16_t skillId, uint8_t targetType, uint32_t targetId) {
        std::vector<uint8_t> message;
        
        // Message type: Skill Use (0x0F)
        message.push_back(0x0F);
        
        // Skill ID (2 bytes, little-endian)
        message.push_back(skillId & 0xFF);
        message.push_back((skillId >> 8) & 0xFF);
        
        // Target Type (1 byte)
        message.push_back(targetType);
        
        // Target ID (4 bytes, little-endian)
        message.push_back(targetId & 0xFF);
        message.push_back((targetId >> 8) & 0xFF);
        message.push_back((targetId >> 16) & 0xFF);
        message.push_back((targetId >> 24) & 0xFF);
        
        return message;
    }
    
    // Mock game client for testing
    class MockGameClient : public GameClient {
    public:
        MockGameClient(GameServer& server, 
                       std::shared_ptr<core::Config> config,
                       std::shared_ptr<database::DatabaseManager> dbManager)
            : GameClient(boost::asio::ip::tcp::socket(m_ioContext), server, config, dbManager),
              m_lastSentMessage(0),
              m_messageQueue() {
        }
        
        // Override to capture sent messages
        void sendMessage(const std::vector<uint8_t>& message) override {
            m_lastSentMessage = message;
            m_messageQueue.push_back(message);
        }
        
        // Process a message directly
        bool processTestMessage(const std::vector<uint8_t>& message) {
            try {
                processMessage(message);
                return true;
            }
            catch (...) {
                return false;
            }
        }
        
        // Get the last sent message
        std::vector<uint8_t> getLastSentMessage() const {
            return m_lastSentMessage;
        }
        
        // Check if any message of the given type was received
        bool receivedMessageOfType(uint8_t messageType) const {
            for (const auto& message : m_messageQueue) {
                if (!message.empty() && message[0] == messageType) {
                    return true;
                }
            }
            return false;
        }
        
        // Setup test character
        void setupTestCharacter(uint32_t characterId, const std::string& characterName, int characterClass) {
            setLogin("testplayer");
            
            database::Character character;
            character.id = characterId;
            character.name = characterName;
            character.classId = characterClass;
            character.level = 10;
            
            setCharacter(character);
        }
        
    private:
        boost::asio::io_context m_ioContext;
        std::vector<uint8_t> m_lastSentMessage;
        std::vector<std::vector<uint8_t>> m_messageQueue;
    };
    
    // Test game session for testing
    class TestGameSession : public GameSession {
    public:
        TestGameSession(std::shared_ptr<core::Config> config,
                        std::shared_ptr<database::DatabaseManager> dbManager)
            : GameSession(1, config, dbManager) {
            init();
        }
        
        // Make update method public for testing
        using GameSession::update;
    };
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<GameServer> m_gameServer;
};

// Test player joining a session
TEST_F(PlayerTest, JoinSession) {
    // Create test session
    auto gameSession = std::make_shared<TestGameSession>(m_config, m_dbManager);
    
    // Create mock client
    MockGameClient client(*m_gameServer, m_config, m_dbManager);
    client.setupTestCharacter(1, "TestChar", 1);
    
    // Client should start with no session
    EXPECT_EQ(client.getGameSession(), nullptr);
    
    // Add client to session
    EXPECT_TRUE(gameSession->addClient(std::make_shared<MockGameClient>(client)));
    
    // Session should have one client
    EXPECT_EQ(gameSession->getClientCount(), 1);
    
    // Client should have a session now
    client.setGameSession(gameSession);
    EXPECT_NE(client.getGameSession(), nullptr);
}

// Test player leaving a session
TEST_F(PlayerTest, LeaveSession) {
    // Create test session
    auto gameSession = std::make_shared<TestGameSession>(m_config, m_dbManager);
    
    // Create mock client
    auto client = std::make_shared<MockGameClient>(*m_gameServer, m_config, m_dbManager);
    client->setupTestCharacter(1, "TestChar", 1);
    
    // Add client to session
    EXPECT_TRUE(gameSession->addClient(client));
    client->setGameSession(gameSession);
    
    // Session should have one client
    EXPECT_EQ(gameSession->getClientCount(), 1);
    
    // Remove client from session
    gameSession->removeClient(client);
    
    // Session should be empty
    EXPECT_EQ(gameSession->getClientCount(), 0);
    EXPECT_TRUE(gameSession->isEmpty());
}

// Test multiple players in a session
TEST_F(PlayerTest, MultiplePlayersInSession) {
    // Create test session
    auto gameSession = std::make_shared<TestGameSession>(m_config, m_dbManager);
    
    // Create mock clients
    auto client1 = std::make_shared<MockGameClient>(*m_gameServer, m_config, m_dbManager);
    client1->setupTestCharacter(1, "Player1", 1);
    
    auto client2 = std::make_shared<MockGameClient>(*m_gameServer, m_config, m_dbManager);
    client2->setupTestCharacter(2, "Player2", 3);
    
    // Add clients to session
    EXPECT_TRUE(gameSession->addClient(client1));
    client1->setGameSession(gameSession);
    
    EXPECT_TRUE(gameSession->addClient(client2));
    client2->setGameSession(gameSession);
    
    // Session should have two clients
    EXPECT_EQ(gameSession->getClientCount(), 2);
    
    // Broadcast a message to all clients
    std::vector<uint8_t> broadcastMsg = {0x0A, 0x01, 0x02, 0x03}; // Dummy world state message
    gameSession->broadcastMessage(broadcastMsg);
    
    // Both clients should receive the message
    EXPECT_EQ(client1->getLastSentMessage(), broadcastMsg);
    EXPECT_EQ(client2->getLastSentMessage(), broadcastMsg);
    
    // Remove one client
    gameSession->removeClient(client1);
    
    // Session should have one client left
    EXPECT_EQ(gameSession->getClientCount(), 1);
    
    // Broadcast another message
    std::vector<uint8_t> broadcastMsg2 = {0x0A, 0x04, 0x05, 0x06};
    gameSession->broadcastMessage(broadcastMsg2);
    
    // Only client2 should receive the new message
    EXPECT_NE(client1->getLastSentMessage(), broadcastMsg2);
    EXPECT_EQ(client2->getLastSentMessage(), broadcastMsg2);
}

// Test session difficulty and game mode
TEST_F(PlayerTest, SessionDifficultyAndMode) {
    // Create test session
    auto gameSession = std::make_shared<TestGameSession>(m_config, m_dbManager);
    
    // Check default difficulty and mode
    EXPECT_EQ(gameSession->getDifficulty(), GameDifficulty::Normal);
    EXPECT_EQ(gameSession->getGameMode(), GameMode::Campaign);
    
    // Change difficulty and mode
    gameSession->setDifficulty(GameDifficulty::Torment1);
    gameSession->setGameMode(GameMode::Adventure);
    
    // Check new difficulty and mode
    EXPECT_EQ(gameSession->getDifficulty(), GameDifficulty::Torment1);
    EXPECT_EQ(gameSession->getGameMode(), GameMode::Adventure);
}

// Test session update
TEST_F(PlayerTest, SessionUpdate) {
    // Create test session
    auto gameSession = std::make_shared<TestGameSession>(m_config, m_dbManager);
    
    // Create mock client
    auto client = std::make_shared<MockGameClient>(*m_gameServer, m_config, m_dbManager);
    client->setupTestCharacter(1, "TestChar", 1);
    
    // Add client to session
    EXPECT_TRUE(gameSession->addClient(client));
    client->setGameSession(gameSession);
    
    // Update session
    gameSession->update(0.016f); // 16ms frame time
    
    // Update multiple times
    for (int i = 0; i < 10; i++) {
        gameSession->update(0.016f);
    }
    
    // Session should still have one client
    EXPECT_EQ(gameSession->getClientCount(), 1);
}

} // namespace test
} // namespace game_server
} // namespace d3server
```

Let me try again with the player_tests.cpp file:

```cpp
#include <gtest/gtest.h>
#include "game_server/game_client.h"
#include "game_server/game_server.h"
#include "game_server/game_session.h"
#include "core/config.h"
#include "database/database_manager.h"
#include <memory>
#include <vector>
#include <string>

namespace d3server {
namespace game_server {
namespace test {

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Setup test database
        setupTestDatabase();
        
        // Create game server
        m_gameServer = std::make_shared<GameServer>(m_config, m_dbManager);
        ASSERT_TRUE(m_gameServer->init());
    }
    
    void TearDown() override {
        m_gameServer.reset();
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Setup test database with test accounts and characters
    void setupTestDatabase() {
        // Create test accounts
        ASSERT_TRUE(m_dbManager->createAccount("player1", "player1@example.com", "password123"));
        ASSERT_TRUE(m_dbManager->createAccount("player2", "player2@example.com", "password123"));
        
        // Create characters
        database::Character char1;
        char1.name = "Barbarian1";
        char1.classId = 1; // Barbarian
        char1.level = 10;
        ASSERT_TRUE(m_dbManager->createCharacter("player1", char1));
        
        database::Character char2;
        char2.name = "Wizard1";
        char2.classId = 3; // Wizard
        char2.level = 15;
        ASSERT_TRUE(m_dbManager->createCharacter("player2", char2));
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<GameServer> m_gameServer;
};

// Test session difficulty and game mode
TEST_F(PlayerTest, SessionDifficultyAndMode) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Check default difficulty and mode
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Normal));
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Campaign));
    
    // Change difficulty and mode
    session->setDifficulty(GameDifficulty::Torment1);
    session->setGameMode(GameMode::Adventure);
    
    // Check updated difficulty and mode
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Torment1));
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Adventure));
}

// Test empty session
TEST_F(PlayerTest, EmptySession) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Session should be empty initially
    EXPECT_TRUE(session->isEmpty());
    EXPECT_EQ(session->getClientCount(), 0);
    EXPECT_EQ(session->getSessionId(), 1);
}

// Test game server operations
TEST_F(PlayerTest, GameServerOperations) {
    // Game server should be initialized
    EXPECT_TRUE(m_gameServer->isRunning());
    
    // Should have no clients initially
    EXPECT_EQ(m_gameServer->getClientCount(), 0);
    
    // Should have no sessions initially
    EXPECT_EQ(m_gameServer->getSessionCount(), 0);
}

} // namespace test
} // namespace game_server
} // namespace d3server
```

Now, let's create tests for the world functionality in the game server:

```cpp
#include <gtest/gtest.h>
#include "game_server/game_session.h"
#include "core/config.h"
#include "database/database_manager.h"
#include <memory>

namespace d3server {
namespace game_server {
namespace test {

class WorldTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
};

// Test different game modes
TEST_F(WorldTest, GameModes) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Test Campaign mode
    session->setGameMode(GameMode::Campaign);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Campaign));
    
    // Test Adventure mode
    session->setGameMode(GameMode::Adventure);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Adventure));
    
    // Test Rift mode
    session->setGameMode(GameMode::Rift);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Rift));
    
    // Test Greater Rift mode
    session->setGameMode(GameMode::GreaterRift);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::GreaterRift));
}

// Test difficulty levels
TEST_F(WorldTest, DifficultyLevels) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Test Normal difficulty
    session->setDifficulty(GameDifficulty::Normal);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Normal));
    
    // Test Hard difficulty
    session->setDifficulty(GameDifficulty::Hard);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Hard));
    
    // Test Expert difficulty
    session->setDifficulty(GameDifficulty::Expert);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Expert));
    
    // Test Master difficulty
    session->setDifficulty(GameDifficulty::Master);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Master));
    
    // Test Torment difficulties
    session->setDifficulty(GameDifficulty::Torment1);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Torment1));
    
    session->setDifficulty(GameDifficulty::Torment6);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Torment6));
}

// Test multiple game sessions
TEST_F(WorldTest, MultipleSessions) {
    // Create multiple game sessions
    auto session1 = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session1->init());
    
    auto session2 = std::make_shared<GameSession>(2, m_config, m_dbManager);
    ASSERT_TRUE(session2->init());
    
    auto session3 = std::make_shared<GameSession>(3, m_config, m_dbManager);
    ASSERT_TRUE(session3->init());
    
    // Each session should have a unique ID
    EXPECT_EQ(session1->getSessionId(), 1);
    EXPECT_EQ(session2->getSessionId(), 2);
    EXPECT_EQ(session3->getSessionId(), 3);
    
    // Set different modes and difficulties
    session1->setGameMode(GameMode::Campaign);
    session1->setDifficulty(GameDifficulty::Normal);
    
    session2->setGameMode(GameMode::Adventure);
    session2->setDifficulty(GameDifficulty::Hard);
    
    session3->setGameMode(GameMode::Rift);
    session3->setDifficulty(GameDifficulty::Torment1);
    
    // Each session should maintain its own state
    EXPECT_EQ(static_cast<int>(session1->getGameMode()), static_cast<int>(GameMode::Campaign));
    EXPECT_EQ(static_cast<int>(session1->getDifficulty()), static_cast<int>(GameDifficulty::Normal));
    
    EXPECT_EQ(static_cast<int>(session2->getGameMode()), static_cast<int>(GameMode::Adventure));
    EXPECT_EQ(static_cast<int>(session2->getDifficulty()), static_cast<int>(GameDifficulty::Hard));
    
    EXPECT_EQ(static_cast<int>(session3->getGameMode()), static_cast<int>(GameMode::Rift));
    EXPECT_EQ(static_cast<int>(session3->getDifficulty()), static_cast<int>(GameDifficulty::Torment1));
}

// Test session updates
TEST_F(WorldTest, SessionUpdates) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Check that session can be updated multiple times
    for (int i = 0; i < 10; i++) {
        session->update(0.016f); // 16ms frame time
    }
    
    // Session should still be in a valid state
    EXPECT_TRUE(session->isEmpty());
    EXPECT_EQ(session->getClientCount(), 0);
}

} // namespace test
} // namespace game_server
} // namespace d3server
```

Let's also create database tests for the account DAO:

```cpp
#include <gtest/gtest.h>
#include "core/logger.h"
#include "utils/debug.h"

int main(int argc, char** argv) {
    // Initialize the logger for tests
    d3server::core::Logger::getInstance().init("database_test.log", true);
    
    // Turn off debug output during tests by default
    d3server::utils::Debug::setEnabled(false);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    return result;
}
```

Now, let's create tests for the account DAO:

```cpp
#include <gtest/gtest.h>
#include "database/database_manager.h"
#include "core/config.h"
#include <memory>
#include <string>

namespace d3server {
namespace database {
namespace test {

class AccountDaoTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<DatabaseManager> m_dbManager;
};

// Test account creation
TEST_F(AccountDaoTest, CreateAccount) {
    // Create test account
    EXPECT_TRUE(m_dbManager->createAccount("testuser", "test@example.com", "password123"));
    
    // Verify account exists
    EXPECT_TRUE(m_dbManager->accountExists("testuser"));
    
    // Attempt to create duplicate account (should fail)
    EXPECT_FALSE(m_dbManager->createAccount("testuser", "another@example.com", "password456"));
}

// Test retrieving account
TEST_F(AccountDaoTest, GetAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("getuser", "get@example.com", "password123"));
    
    // Retrieve account
    Account account = m_dbManager->getAccount("getuser");
    
    // Verify account data
    EXPECT_EQ(account.login, "getuser");
    EXPECT_EQ(account.email, "get@example.com");
    EXPECT_FALSE(account.banned);
    EXPECT_EQ(account.characterCount, 0);
}

// Test updating account
TEST_F(AccountDaoTest, UpdateAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("updateuser", "original@example.com", "password123"));
    
    // Update email
    EXPECT_TRUE(m_dbManager->updateAccount("updateuser", "updated@example.com", ""));
    
    // Verify update
    Account account = m_dbManager->getAccount("updateuser");
    EXPECT_EQ(account.email, "updated@example.com");
    
    // Update password
    EXPECT_TRUE(m_dbManager->updateAccount("updateuser", "", "newpassword"));
    
    // Verify password update (by authenticating)
    EXPECT_TRUE(m_dbManager->verifyAccountPassword("updateuser", "newpassword"));
    EXPECT_FALSE(m_dbManager->verifyAccountPassword("updateuser", "password123"));
}

// Test banning account
TEST_F(AccountDaoTest, BanAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("banuser", "ban@example.com", "password123"));
    
    // Ban account
    EXPECT_TRUE(m_dbManager->setBanStatus("banuser", true, "Test ban reason"));
    
    // Verify ban status
    EXPECT_TRUE(m_dbManager->isAccountBanned("banuser"));
    
    // Unban account
    EXPECT_TRUE(m_dbManager->setBanStatus("banuser", false, ""));
    
    // Verify unbanned
    EXPECT_FALSE(m_dbManager->isAccountBanned("banuser"));
}

// Test deleting account
TEST_F(AccountDaoTest, DeleteAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("deleteuser", "delete@example.com", "password123"));
    
    // Verify account exists
    EXPECT_TRUE(m_dbManager->accountExists("deleteuser"));
    
    // Delete account
    EXPECT_TRUE(m_dbManager->deleteAccount("deleteuser"));
    
    // Verify account no longer exists
    EXPECT_FALSE(m_dbManager->accountExists("deleteuser"));
    
    // Attempt to delete non-existent account (should fail)
    EXPECT_FALSE(m_dbManager->deleteAccount("nonexistent"));
}

// Test password verification
TEST_F(AccountDaoTest, PasswordVerification) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("passuser", "pass@example.com", "correctpassword"));
    
    // Test password verification
    EXPECT_TRUE(m_dbManager->verifyAccountPassword("passuser", "correctpassword"));
    EXPECT_FALSE(m_dbManager->verifyAccountPassword("passuser", "wrongpassword"));
    EXPECT_FALSE(m_dbManager->verifyAccountPassword("nonexistent", "password"));
}

// Test getting all accounts
TEST_F(AccountDaoTest, GetAllAccounts) {
    // Create multiple test accounts
    ASSERT_TRUE(m_dbManager->createAccount("user1", "user1@example.com", "password1"));
    ASSERT_TRUE(m_dbManager->createAccount("user2", "user2@example.com", "password2"));
    ASSERT_TRUE(m_dbManager->createAccount("user3", "user3@example.com", "password3"));
    
    // Get all accounts
    std::vector<Account> accounts = m_dbManager->getAllAccounts();
    
    // Verify accounts count
    EXPECT_GE(accounts.size(), 3);
    
    // Verify account data (this is a simplistic check)
    bool foundUser1 = false;
    bool foundUser2 = false;
    bool foundUser3 = false;
    
    for (const auto& account : accounts) {
        if (account.login == "user1") foundUser1 = true;
        if (account.login == "user2") foundUser2 = true;
        if (account.login == "user3") foundUser3 = true;
    }
    
    EXPECT_TRUE(foundUser1);
    EXPECT_TRUE(foundUser2);
    EXPECT_TRUE(foundUser3);
}

// Test account count
TEST_F(AccountDaoTest, AccountCount) {
    // Get initial count
    int initialCount = m_dbManager->getAccountCount();
    
    // Create multiple test accounts
    ASSERT_TRUE(m_dbManager->createAccount("countuser1", "count1@example.com", "password1"));
    ASSERT_TRUE(m_dbManager->createAccount("countuser2", "count2@example.com", "password2"));
    
    // Verify count increased
    EXPECT_EQ(m_dbManager->getAccountCount(), initialCount + 2);
    
    // Delete an account
    ASSERT_TRUE(m_dbManager->deleteAccount("countuser1"));
    
    // Verify count decreased
    EXPECT_EQ(m_dbManager->getAccountCount(), initialCount + 1);
}

} // namespace test
} // namespace database
} // namespace d3server
```

Now, let's create tests for the character DAO:

```cpp
#include <gtest/gtest.h>
#include "database/database_manager.h"
#include "core/config.h"
#include <memory>
#include <string>

namespace d3server {
namespace database {
namespace test {

class CharacterDaoTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Create test account
        ASSERT_TRUE(m_dbManager->createAccount("testaccount", "test@example.com", "password123"));
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Helper to create a character
    Character createTestCharacter(const std::string& name, int classId, int level = 1) {
        Character character;
        character.name = name;
        character.classId = classId;
        character.level = level;
        return character;
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<DatabaseManager> m_dbManager;
};

// Test character creation
TEST_F(CharacterDaoTest, CreateCharacter) {
    // Create character
    Character character = createTestCharacter("Barbarian1", 1, 10);
    
    // Add character to account
    EXPECT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Verify character exists
    EXPECT_TRUE(m_dbManager->characterExists("Barbarian1"));
    
    // Attempt to create duplicate character (should fail)
    EXPECT_FALSE(m_dbManager->createCharacter("testaccount", character));
}

// Test getting all characters for an account
TEST_F(CharacterDaoTest, GetAllCharacters) {
    // Create multiple characters
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Barbarian1", 1, 10)));
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Wizard1", 3, 15)));
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Monk1", 2, 5)));
    
    // Get characters
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    
    // Verify character count
    EXPECT_EQ(characters.size(), 3);
    
    // Verify character data (this is a simplistic check)
    bool foundBarbarian = false;
    bool foundWizard = false;
    bool foundMonk = false;
    
    for (const auto& character : characters) {
        if (character.name == "Barbarian1") {
            foundBarbarian = true;
            EXPECT_EQ(character.classId, 1);
            EXPECT_EQ(character.level, 10);
        }
        else if (character.name == "Wizard1") {
            foundWizard = true;
            EXPECT_EQ(character.classId, 3);
            EXPECT_EQ(character.level, 15);
        }
        else if (character.name == "Monk1") {
            foundMonk = true;
            EXPECT_EQ(character.classId, 2);
            EXPECT_EQ(character.level, 5);
        }
    }
    
    EXPECT_TRUE(foundBarbarian);
    EXPECT_TRUE(foundWizard);
    EXPECT_TRUE(foundMonk);
}

// Test getting character count
TEST_F(CharacterDaoTest, GetCharacterCount) {
    // Verify initial count
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), 0);
    
    // Create multiple characters
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Char1", 1)));
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Char2", 2)));
    
    // Verify count
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), 2);
}

// Test max character limit
TEST_F(CharacterDaoTest, MaxCharacterLimit) {
    // Get max characters per account from config
    int maxCharacters = m_config->getServerConfig().maxCharactersPerAccount;
    
    // If max characters is not set or unreasonably high, use a smaller value for testing
    if (maxCharacters <= 0 || maxCharacters > 20) {
        maxCharacters = 5;
        
        // Set max characters in config
        auto serverConfig = m_config->getServerConfig();
        serverConfig.maxCharactersPerAccount = maxCharacters;
        m_config->updateServerConfig(serverConfig);
    }
    
    // Create characters up to the limit
    for (int i = 0; i < maxCharacters; i++) {
        ASSERT_TRUE(m_dbManager->createCharacter(
            "testaccount", 
            createTestCharacter("Character" + std::to_string(i + 1), (i % 7) + 1)
        ));
    }
    
    // Verify count
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), maxCharacters);
    
    // Attempt to create one more character (should fail)
    EXPECT_FALSE(m_dbManager->createCharacter(
        "testaccount", 
        createTestCharacter("ExtraCharacter", 1)
    ));
    
    // Verify count hasn't changed
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), maxCharacters);
}

// Test retrieving character by ID
TEST_F(CharacterDaoTest, GetCharacterById) {
    // Create character
    Character character = createTestCharacter("GetByIdTest", 3, 25);
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Get all characters to find the ID
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    ASSERT_FALSE(characters.empty());
    
    // Find the character we just created
    uint32_t characterId = 0;
    for (const auto& c : characters) {
        if (c.name == "GetByIdTest") {
            characterId = c.id;
            break;
        }
    }
    
    // Verify character ID is valid
    ASSERT_GT(characterId, 0);
    
    // Get character by ID
    Character retrievedCharacter = m_dbManager->getCharacter(characterId);
    
    // Verify character data
    EXPECT_EQ(retrievedCharacter.id, characterId);
    EXPECT_EQ(retrievedCharacter.name, "GetByIdTest");
    EXPECT_EQ(retrievedCharacter.classId, 3);
    EXPECT_EQ(retrievedCharacter.level, 25);
}

// Test updating character
TEST_F(CharacterDaoTest, UpdateCharacter) {
    // Create character
    Character character = createTestCharacter("UpdateTest", 1, 10);
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Get character ID
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    uint32_t characterId = 0;
    for (const auto& c : characters) {
        if (c.name == "UpdateTest") {
            characterId = c.id;
            break;
        }
    }
    ASSERT_GT(characterId, 0);
    
    // Update character
    Character updatedCharacter = m_dbManager->getCharacter(characterId);
    updatedCharacter.level = 20;
    EXPECT_TRUE(m_dbManager->updateCharacter(updatedCharacter));
    
    // Verify update
    Character retrievedCharacter = m_dbManager->getCharacter(characterId);
    EXPECT_EQ(retrievedCharacter.level, 20);
}

// Test deleting character
TEST_F(CharacterDaoTest, DeleteCharacter) {
    // Create character
    Character character = createTestCharacter("DeleteTest", 2, 15);
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Verify character exists
    EXPECT_TRUE(m_dbManager->characterExists("DeleteTest"));
    
    // Get character ID
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    uint32_t characterId = 0;
    for (const auto& c : characters) {
        if (c.name == "DeleteTest") {
            characterId = c.id;
            break;
        }
    }
    ASSERT_GT(characterId, 0);
    
    // Delete character
    EXPECT_TRUE(m_dbManager->deleteCharacter(characterId));
    
    // Verify character no longer exists
    EXPECT_FALSE(m_dbManager->characterExists("DeleteTest"));
    
    // Verify character count decreased
    std::vector<Character> remainingCharacters = m_dbManager->getCharacters("testaccount");
    bool characterFound = false;
    for (const auto& c : remainingCharacters) {
        if (c.name == "DeleteTest") {
            characterFound = true;
            break;
        }
    }
    EXPECT_FALSE(characterFound);
}

} // namespace test
} // namespace database
} // namespace d3server
```

Now we have created a comprehensive Chinese documentation and added unit tests for the major components of the D3Server project. The Chinese documentation provides detailed implementation guidance, notes, and best practices for the development team. The unit tests ensure the code is working correctly and will help maintain quality as the project evolves.

The tests cover:
1. Core components (Config and Logger)
2. Database functionality (Account and Character DAO)
3. REST API (AccountController)
4. Battle.net functionality (Authentication)
5. Game server functionality (Sessions and World)

These tests provide good coverage of the codebase and should help catch regressions during development. The comprehensive Chinese documentation in `docs/implementation_guide_cn.md` provides a clear reference for developers on the project architecture, implementation details, and best practices.

---

**User**

帮我完善下代码和上下文，另外检查下是否有其他问题，我的要求是在原来 C#版本的基础上增加上debug审计，让出现问题的时候，我可以修改对应的代码来兼容，还有请完善下测试和contribute流程，给出对应的中文示例， 方便别人参与进来。

---

**Cursor**

我会帮你完善代码并检查问题，特别是针对调试审计功能和测试贡献流程。

```markdown
# D3Server 贡献指南

感谢您对 D3Server 项目的兴趣！本指南将帮助您了解如何为项目做出贡献，包括代码规范、提交流程、测试要求和调试技巧。

## 目录

- [开发环境设置](#开发环境设置)
- [代码风格](#代码风格)
- [分支管理](#分支管理)
- [提交更改](#提交更改)
- [测试要求](#测试要求)
- [调试指南](#调试指南)
- [文档](#文档)
- [常见问题解答](#常见问题解答)

## 开发环境设置

1. **克隆仓库**

   ```bash
   git clone https://github.com/example/d3server.git
   cd d3server
   ```

2. **安装依赖**

   在 Ubuntu/Debian 系统上：
   ```bash
   sudo apt-get update
   sudo apt-get install -y build-essential cmake libsqlite3-dev libssl-dev libgtest-dev libboost-all-dev
   ```

   在 CentOS/RHEL 系统上：
   ```bash
   sudo yum install -y gcc-c++ cmake sqlite-devel openssl-devel gtest-devel boost-devel
   ```

3. **构建项目**

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

4. **运行测试**

   ```bash
   make test
   ```

## 代码风格

我们使用基于 Google C++ 风格指南的代码规范。以下是主要规则：

1. **命名约定**
   - 类名：使用首字母大写的 PascalCase，例如 `GameServer`
   - 方法和函数：使用小驼峰命名法，例如 `createAccount()`
   - 成员变量：使用 m_ 前缀，例如 `m_dbManager`
   - 常量：使用全大写和下划线，例如 `MAX_PLAYERS`
   - 命名空间：使用小写，例如 `d3server::core`

2. **缩进和格式**
   - 使用 4 个空格缩进（不使用制表符）
   - 行长度限制为 100 个字符
   - 大括号始终使用新行
   - 每个语句后添加分号，即使是单行代码块

3. **注释规范**
   - 所有公共 API 应有 Doxygen 风格的注释
   - 复杂算法或不明显的代码应有解释性注释
   - 使用 `TODO:`、`FIXME:` 等标签标记需要改进的代码

4. **其他规则**
   - 使用 `nullptr` 而不是 `NULL` 或 `0`
   - 包含头文件时使用 `#pragma once` 作为防护
   - 优先使用 `std::unique_ptr` 和 `std::shared_ptr` 而非原始指针
   - 优先使用 C++17 特性，如 structured bindings，`std::optional` 等

## 分支管理

我们使用以下分支模型：

- `master`: 主分支，包含稳定发布版本
- `develop`: 开发分支，用于集成功能
- `feature/*`: 新功能分支
- `bugfix/*`: 错误修复分支
- `hotfix/*`: 紧急修复分支

开发新功能或修复 bug 时，请从 `develop` 分支创建一个新的特性/错误修复分支：

```bash
git checkout develop
git pull
git checkout -b feature/your-feature-name
```

## 提交更改

1. **提交前检查**
   - 确保代码遵循代码风格
   - 确保所有测试通过
   - 确保没有引入新的编译警告

2. **提交信息规范**
   
   使用清晰的提交信息，格式如下：
   ```
   类型(作用域): 简短描述

   详细描述（如果需要）
   
   解决问题 #123
   ```

   类型可以是：
   - `feat`: 新功能
   - `fix`: 错误修复
   - `docs`: 文档更改
   - `style`: 代码风格调整（不影响代码逻辑）
   - `refactor`: 代码重构
   - `perf`: 性能优化
   - `test`: 测试相关
   - `chore`: 构建过程或辅助工具的变动

3. **创建拉取请求（PR）**
   
   推送您的分支并在 GitHub 上创建拉取请求：
   
   ```bash
   git push origin feature/your-feature-name
   ```
   
   PR 标题应遵循与提交信息相同的格式。

4. **代码审查**
   
   所有代码更改必须经过至少一名项目维护者的审查。请耐心等待反馈，并根据反馈进行必要的更改。

## 测试要求

所有新代码都必须包含适当的测试：

1. **单元测试**
   - 每个新功能必须有对应的单元测试
   - 每个修复的bug应有测试以防止回归
   - 使用 Google Test 框架

2. **代码覆盖率**
   - 新代码应至少有 80% 的测试覆盖率
   - 对于核心功能，建议达到 90% 以上的覆盖率

3. **测试结构**
   - 测试应放在 `test/unit/` 目录下
   - 测试文件命名应使用 `{component}_tests.cpp` 格式
   - 测试类应使用 `{Component}Test` 命名

4. **测试示例**

   ```cpp
   #include <gtest/gtest.h>
   #include "core/config.h"
   
   namespace d3server {
   namespace core {
   namespace test {
   
   class ConfigTest : public ::testing::Test {
   protected:
       void SetUp() override {
           m_config = std::make_shared<Config>();
           ASSERT_TRUE(m_config->init(":memory:"));
       }
       
       void TearDown() override {
           m_config.reset();
       }
       
       std::shared_ptr<Config> m_config;
   };
   
   TEST_F(ConfigTest, ServerConfig) {
       auto serverConfig = m_config->getServerConfig();
       EXPECT_FALSE(serverConfig.serverName.empty());
       
       serverConfig.serverName = "TestServer";
       EXPECT_TRUE(m_config->updateServerConfig(serverConfig));
       
       auto updatedConfig = m_config->getServerConfig();
       EXPECT_EQ(updatedConfig.serverName, "TestServer");
   }
   
   } // namespace test
   } // namespace core
   } // namespace d3server
   ```

## 调试指南

项目包含强大的调试工具，有助于快速定位和修复问题：

1. **调试宏**

   项目提供了一系列调试宏，在定义了 `DEBUG_MODE` 后可用：
   
   ```cpp
   DEBUG_LOG("消息内容");               // 记录一般调试消息
   DEBUG_FUNCTION_ENTER();             // 记录函数进入
   DEBUG_FUNCTION_EXIT();              // 记录函数退出
   DEBUG_VARIABLE(variableName);       // 记录变量值
   DEBUG_TIMER_START("操作名称");       // 开始计时
   DEBUG_TIMER_STOP();                 // 停止计时并记录耗时
   ```

2. **启用调试模式**

   在 CMake 中启用调试模式：
   
   ```bash
   cmake -DDEBUG_MODE=ON ..
   ```

3. **调试日志**

   调试日志包含详细信息，如：
   - 时间戳（精确到毫秒）
   - 线程 ID
   - 源文件名和行号
   - 函数名
   - 上下文信息（如变量值）

4. **性能分析**

   使用定时器分析性能瓶颈：
   
   ```cpp
   void someExpensiveOperation() {
       DEBUG_TIMER_START("ExpensiveOperation");
       
       // 执行耗时操作...
       
       DEBUG_TIMER_STOP(); // 输出操作耗时
   }
   ```

5. **自定义调试回调**

   可以注册自定义回调来处理调试信息：
   
   ```cpp
   utils::Debug::getInstance().setLogCallback([](const std::string& message) {
       // 处理调试消息，例如发送到远程监控系统
   });
   ```

## 文档

代码更改应同时更新相关文档：

1. **API 文档**
   - 更新 API 端点信息
   - 包含请求和响应示例

2. **设计文档**
   - 更新架构图
   - 记录设计决策和权衡

3. **中文文档**
   - 所有核心文档应同时提供中文版本
   - 中文文档文件使用 `*_cn.md` 命名

## 常见问题解答

**Q: 如何运行特定的测试？**

A: 您可以使用 Google Test 过滤器运行特定测试：

```bash
./tests/unit_tests --gtest_filter=ConfigTest.ServerConfig
```

**Q: 如何处理第三方依赖？**

A: 首选使用系统包管理器安装依赖。对于特殊需求，可以使用 git submodule 或 CMake FetchContent。

**Q: 如何贡献文档而非代码？**

A: 文档贡献与代码贡献流程相同。创建分支，更新文档，提交 PR。

**Q: 如何报告 bug？**

A: 在 GitHub issue 跟踪器中创建新 issue，提供以下信息：
- 问题描述
- 重现步骤
- 期望行为
- 实际行为
- 环境信息（操作系统、编译器版本等）
- 如可能，附上日志和截图

---

感谢您的贡献！如有任何疑问，请联系项目维护者。
```

```cpp
#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <functional>
#include <fstream>
#include <memory>

namespace d3server {
namespace utils {

/**
 * @brief 操作类型枚举，用于标识不同类型的操作
 */
enum class AuditActionType {
    AUTHENTICATION,     // 认证操作
    ACCOUNT_MANAGEMENT, // 账号管理操作
    CHARACTER_MANAGEMENT, // 角色管理操作
    GAME_SESSION,       // 游戏会话操作
    CONFIGURATION,      // 配置更改
    ADMIN_ACTION,       // 管理员操作
    SECURITY,           // 安全相关操作
    DATABASE,           // 数据库操作
    NETWORK,            // 网络连接操作
    CUSTOM              // 自定义操作
};

/**
 * @brief 操作结果枚举，表示操作成功或失败
 */
enum class AuditResult {
    SUCCESS,            // 操作成功
    FAILURE,            // 操作失败
    WARNING,            // 操作完成但有警告
    UNAUTHORIZED        // 未授权的操作
};

/**
 * @brief 审计日志条目结构
 */
struct AuditEntry {
    std::chrono::system_clock::time_point timestamp; // 操作时间戳
    std::string userId;                              // 用户标识
    std::string ipAddress;                           // IP地址
    AuditActionType actionType;                      // 操作类型
    std::string actionDescription;                   // 操作描述
    AuditResult result;                              // 操作结果
    std::string details;                             // 详细信息
    std::string sourceLocation;                      // 源代码位置
};

/**
 * @brief 将操作类型转换为字符串
 * @param type 操作类型
 * @return 操作类型的字符串表示
 */
std::string auditActionTypeToString(AuditActionType type);

/**
 * @brief 将操作结果转换为字符串
 * @param result 操作结果
 * @return 操作结果的字符串表示
 */
std::string auditResultToString(AuditResult result);

/**
 * @brief 审计日志系统类
 */
class AuditLog {
public:
    /**
     * @brief 获取审计日志系统的单例实例
     * @return 审计日志系统引用
     */
    static AuditLog& getInstance();

    /**
     * @brief 初始化审计日志系统
     * @param logFilePath 审计日志文件路径
     * @param maxEntries 内存中保存的最大条目数
     * @param enabled 是否启用审计日志
     * @return 初始化是否成功
     */
    bool init(const std::string& logFilePath = "audit.log", size_t maxEntries = 1000, bool enabled = true);

    /**
     * @brief 记录审计操作
     * @param userId 用户标识
     * @param ipAddress IP地址
     * @param actionType 操作类型
     * @param actionDescription 操作描述
     * @param result 操作结果
     * @param details 详细信息
     * @param file 源文件
     * @param line 源码行号
     * @param function 函数名
     */
    void logAction(
        const std::string& userId,
        const std::string& ipAddress,
        AuditActionType actionType,
        const std::string& actionDescription,
        AuditResult result,
        const std::string& details,
        const std::string& file,
        int line,
        const std::string& function
    );

    /**
     * @brief 获取最近的审计条目
     * @param count 要获取的条目数量
     * @return 审计条目向量
     */
    std::vector<AuditEntry> getRecentEntries(size_t count);

    /**
     * @brief 获取满足特定条件的审计条目
     * @param filter 过滤函数，接收一个AuditEntry并返回bool
     * @return 满足条件的审计条目向量
     */
    std::vector<AuditEntry> getEntriesByFilter(std::function<bool(const AuditEntry&)> filter);

    /**
     * @brief 导出审计日志到CSV文件
     * @param filePath 导出文件路径
     * @return 导出是否成功
     */
    bool exportToCSV(const std::string& filePath);

    /**
     * @brief 设置审计回调函数
     * @param callback 回调函数，接收一个AuditEntry
     */
    void setAuditCallback(std::function<void(const AuditEntry&)> callback);

    /**
     * @brief 启用或禁用审计日志
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);

    /**
     * @brief 检查审计日志是否启用
     * @return 是否启用
     */
    bool isEnabled() const;

    /**
     * @brief 清理并关闭审计日志
     */
    void shutdown();

private:
    AuditLog(); // 私有构造函数
    ~AuditLog();
    AuditLog(const AuditLog&) = delete;
    AuditLog& operator=(const AuditLog&) = delete;

    /**
     * @brief 将审计条目写入文件
     * @param entry 审计条目
     */
    void writeToFile(const AuditEntry& entry);

    /**
     * @brief 格式化审计条目为字符串
     * @param entry 审计条目
     * @return 格式化的字符串
     */
    std::string formatEntry(const AuditEntry& entry);

    std::mutex m_mutex;
    std::ofstream m_logFile;
    std::vector<AuditEntry> m_entries;
    std::function<void(const AuditEntry&)> m_callback;
    size_t m_maxEntries;
    bool m_enabled;
    bool m_initialized;
    std::string m_logFilePath;
};

// 方便使用的宏
#define AUDIT_LOG(userId, ipAddress, actionType, actionDescription, result, details) \
    d3server::utils::AuditLog::getInstance().logAction( \
        userId, ipAddress, actionType, actionDescription, result, details, __FILE__, __LINE__, __FUNCTION__)

} // namespace utils
} // namespace d3server
```

```cpp
#include "utils/audit.h"
#include "utils/debug.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace d3server {
namespace utils {

std::string auditActionTypeToString(AuditActionType type) {
    switch (type) {
        case AuditActionType::AUTHENTICATION:      return "AUTHENTICATION";
        case AuditActionType::ACCOUNT_MANAGEMENT:  return "ACCOUNT_MANAGEMENT";
        case AuditActionType::CHARACTER_MANAGEMENT: return "CHARACTER_MANAGEMENT";
        case AuditActionType::GAME_SESSION:        return "GAME_SESSION";
        case AuditActionType::CONFIGURATION:       return "CONFIGURATION";
        case AuditActionType::ADMIN_ACTION:        return "ADMIN_ACTION";
        case AuditActionType::SECURITY:            return "SECURITY";
        case AuditActionType::DATABASE:            return "DATABASE";
        case AuditActionType::NETWORK:             return "NETWORK";
        case AuditActionType::CUSTOM:              return "CUSTOM";
        default:                                   return "UNKNOWN";
    }
}

std::string auditResultToString(AuditResult result) {
    switch (result) {
        case AuditResult::SUCCESS:       return "SUCCESS";
        case AuditResult::FAILURE:       return "FAILURE";
        case AuditResult::WARNING:       return "WARNING";
        case AuditResult::UNAUTHORIZED:  return "UNAUTHORIZED";
        default:                         return "UNKNOWN";
    }
}

AuditLog::AuditLog() 
    : m_maxEntries(1000), 
      m_enabled(false),
      m_initialized(false) {
}

AuditLog::~AuditLog() {
    shutdown();
}

AuditLog& AuditLog::getInstance() {
    static AuditLog instance;
    return instance;
}

bool AuditLog::init(const std::string& logFilePath, size_t maxEntries, bool enabled) {
    DEBUG_FUNCTION_ENTER();
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 如果已经初始化，先关闭现有文件
    if (m_initialized && m_logFile.is_open()) {
        m_logFile.close();
    }
    
    m_logFilePath = logFilePath;
    m_maxEntries = maxEntries;
    m_enabled = enabled;
    
    // 创建目录（如果不存在）
    try {
        std::filesystem::path logPath(logFilePath);
        if (logPath.has_parent_path()) {
            std::filesystem::create_directories(logPath.parent_path());
        }
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Failed to create audit log directory: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
    
    // 打开日志文件
    m_logFile.open(logFilePath, std::ios::out | std::ios::app);
    if (!m_logFile.is_open()) {
        DEBUG_LOG("Failed to open audit log file: " + logFilePath);
        DEBUG_FUNCTION_EXIT();
        return false;
    }
    
    // 写入标题（如果是新文件）
    if (m_logFile.tellp() == 0) {
        m_logFile << "Timestamp,User ID,IP Address,Action Type,Action Description,Result,Details,Source Location" << std::endl;
    }
    
    m_initialized = true;
    
    // 记录初始化日志
    AuditEntry initEntry;
    initEntry.timestamp = std::chrono::system_clock::now();
    initEntry.userId = "SYSTEM";
    initEntry.ipAddress = "127.0.0.1";
    initEntry.actionType = AuditActionType::CONFIGURATION;
    initEntry.actionDescription = "Audit Log Initialized";
    initEntry.result = AuditResult::SUCCESS;
    initEntry.details = "Max entries: " + std::to_string(maxEntries) + ", Enabled: " + (enabled ? "true" : "false");
    initEntry.sourceLocation = __FILE__ + std::string(":") + std::to_string(__LINE__) + " " + __FUNCTION__;
    
    writeToFile(initEntry);
    if (m_entries.size() >= m_maxEntries) {
        m_entries.erase(m_entries.begin());
    }
    m_entries.push_back(initEntry);
    
    DEBUG_FUNCTION_EXIT();
    return true;
}

void AuditLog::logAction(
    const std::string& userId,
    const std::string& ipAddress,
    AuditActionType actionType,
    const std::string& actionDescription,
    AuditResult result,
    const std::string& details,
    const std::string& file,
    int line,
    const std::string& function) {
    
    if (!m_enabled || !m_initialized) {
        return;
    }
    
    DEBUG_FUNCTION_ENTER();
    
    // 创建审计条目
    AuditEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.userId = userId;
    entry.ipAddress = ipAddress;
    entry.actionType = actionType;
    entry.actionDescription = actionDescription;
    entry.result = result;
    entry.details = details;
    
    // 格式化源代码位置
    std::filesystem::path filePath(file);
    entry.sourceLocation = filePath.filename().string() + ":" + std::to_string(line) + " " + function;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 写入文件
        writeToFile(entry);
        
        // 添加到内存缓存
        if (m_entries.size() >= m_maxEntries) {
            m_entries.erase(m_entries.begin());
        }
        m_entries.push_back(entry);
    }
    
    // 调用回调函数（如果设置了）
    if (m_callback) {
        try {
            m_callback(entry);
        }
        catch (const std::exception& e) {
            DEBUG_LOG("Exception in audit callback: " + std::string(e.what()));
        }
    }
    
    DEBUG_FUNCTION_EXIT();
}

std::vector<AuditEntry> AuditLog::getRecentEntries(size_t count) {
    DEBUG_FUNCTION_ENTER();
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_enabled || !m_initialized) {
        DEBUG_FUNCTION_EXIT();
        return {};
    }
    
    if (count >= m_entries.size()) {
        DEBUG_FUNCTION_EXIT();
        return m_entries;
    }
    
    // 返回最近的条目
    auto startIt = m_entries.end() - count;
    std::vector<AuditEntry> result(startIt, m_entries.end());
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::vector<AuditEntry> AuditLog::getEntriesByFilter(std::function<bool(const AuditEntry&)> filter) {
    DEBUG_FUNCTION_ENTER();
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_enabled || !m_initialized) {
        DEBUG_FUNCTION_EXIT();
        return {};
    }
    
    std::vector<AuditEntry> result;
    for (const auto& entry : m_entries) {
        try {
            if (filter(entry)) {
                result.push_back(entry);
            }
        }
        catch (const std::exception& e) {
            DEBUG_LOG("Exception in filter function: " + std::string(e.what()));
        }
    }
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

bool AuditLog::exportToCSV(const std::string& filePath) {
    DEBUG_FUNCTION_ENTER();
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_enabled || !m_initialized) {
        DEBUG_FUNCTION_EXIT();
        return false;
    }
    
    try {
        // 创建目录（如果不存在）
        std::filesystem::path path(filePath);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        
        // 打开CSV文件
        std::ofstream csvFile(filePath);
        if (!csvFile.is_open()) {
            DEBUG_LOG("Failed to open CSV file for export: " + filePath);
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // 写入CSV头
        csvFile << "Timestamp,User ID,IP Address,Action Type,Action Description,Result,Details,Source Location" << std::endl;
        
        // 写入所有条目
        for (const auto& entry : m_entries) {
            auto timeT = std::chrono::system_clock::to_time_t(entry.timestamp);
            auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                entry.timestamp.time_since_epoch()) % 1000;
            
            std::stringstream timeStr;
            timeStr << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S") 
                    << '.' << std::setfill('0') << std::setw(3) << timeMs.count();
            
            csvFile << timeStr.str() << ","
                   << "\"" << entry.userId << "\","
                   << "\"" << entry.ipAddress << "\","
                   << "\"" << auditActionTypeToString(entry.actionType) << "\","
                   << "\"" << entry.actionDescription << "\","
                   << "\"" << auditResultToString(entry.result) << "\","
                   << "\"" << entry.details << "\","
                   << "\"" << entry.sourceLocation << "\""
                   << std::endl;
        }
        
        csvFile.close();
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception during CSV export: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void AuditLog::setAuditCallback(std::function<void(const AuditEntry&)> callback) {
    DEBUG_FUNCTION_ENTER();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callback = callback;
    DEBUG_FUNCTION_EXIT();
}

void AuditLog::setEnabled(bool enabled) {
    DEBUG_FUNCTION_ENTER();
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_enabled != enabled) {
        m_enabled = enabled;
        
        // 记录状态变更
        if (m_initialized) {
            AuditEntry entry;
            entry.timestamp = std::chrono::system_clock::now();
            entry.userId = "SYSTEM";
            entry.ipAddress = "127.0.0.1";
            entry.actionType = AuditActionType::CONFIGURATION;
            entry.actionDescription = "Audit Log Status Changed";
            entry.result = AuditResult::SUCCESS;
            entry.details = "Enabled: " + std::string(enabled ? "true" : "false");
            entry.sourceLocation = __FILE__ + std::string(":") + std::to_string(__LINE__) + " " + __FUNCTION__;
            
            // 如果是启用状态，记录这个事件
            if (enabled) {
                writeToFile(entry);
                if (m_entries.size() >= m_maxEntries) {
                    m_entries.erase(m_entries.begin());
                }
                m_entries.push_back(entry);
            }
        }
    }
    
    DEBUG_FUNCTION_EXIT();
}

bool AuditLog::isEnabled() const {
    return m_enabled && m_initialized;
}

void AuditLog::shutdown() {
    DEBUG_FUNCTION_ENTER();
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        // 记录关闭事件
        if (m_enabled && m_logFile.is_open()) {
            AuditEntry entry;
            entry.timestamp = std::chrono::system_clock::now();
            entry.userId = "SYSTEM";
            entry.ipAddress = "127.0.0.1";
            entry.actionType = AuditActionType::CONFIGURATION;
            entry.actionDescription = "Audit Log Shutdown";
            entry.result = AuditResult::SUCCESS;
            entry.details = "Normal shutdown";
            entry.sourceLocation = __FILE__ + std::string(":") + std::to_string(__LINE__) + " " + __FUNCTION__;
            
            writeToFile(entry);
        }
        
        // 关闭文件
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
        
        // 清空缓存
        m_entries.clear();
        m_initialized = false;
    }
    
    DEBUG_FUNCTION_EXIT();
}

void AuditLog::writeToFile(const AuditEntry& entry) {
    if (!m_logFile.is_open()) {
        return;
    }
    
    try {
        // 将时间戳转换为可读格式
        auto timeT = std::chrono::system_clock::to_time_t(entry.timestamp);
        auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            entry.timestamp.time_since_epoch()) % 1000;
        
        std::stringstream timeStr;
        timeStr << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S") 
                << '.' << std::setfill('0') << std::setw(3) << timeMs.count();
        
        // 写入CSV格式
        m_logFile << timeStr.str() << ","
                 << "\"" << entry.userId << "\","
                 << "\"" << entry.ipAddress << "\","
                 << "\"" << auditActionTypeToString(entry.actionType) << "\","
                 << "\"" << entry.actionDescription << "\","
                 << "\"" << auditResultToString(entry.result) << "\","
                 << "\"" << entry.details << "\","
                 << "\"" << entry.sourceLocation << "\""
                 << std::endl;
        
        // 确保写入磁盘
        m_logFile.flush();
    }
    catch (const std::exception& e) {
        std::cerr << "Error writing to audit log: " << e.what() << std::endl;
    }
}

std::string AuditLog::formatEntry(const AuditEntry& entry) {
    // 将时间戳转换为可读格式
    auto timeT = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S") 
       << '.' << std::setfill('0') << std::setw(3) << timeMs.count()
       << " [" << auditResultToString(entry.result) << "] "
       << "[" << auditActionTypeToString(entry.actionType) << "] "
       << "User: " << entry.userId << " "
       << "IP: " << entry.ipAddress << " "
       << "Action: " << entry.actionDescription << " "
       << "Details: " << entry.details << " "
       << "Location: " << entry.sourceLocation;
    
    return ss.str();
}

} // namespace utils
} // namespace d3server
```

```cpp
#include <gtest/gtest.h>
#include "utils/audit.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <functional>
#include <thread>
#include <chrono>

namespace d3server {
namespace utils {
namespace test {

class AuditTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试审计日志文件路径
        m_auditFilePath = "test_audit.log";
        
        // 初始化审计日志系统
        AuditLog::getInstance().init(m_auditFilePath, 100, true);
    }
    
    void TearDown() override {
        // 关闭审计日志系统
        AuditLog::getInstance().shutdown();
        
        // 清理测试审计日志文件
        if (std::filesystem::exists(m_auditFilePath)) {
            std::filesystem::remove(m_auditFilePath);
        }
    }
    
    // 辅助函数：检查日志文件是否包含特定消息
    bool logContainsMessage(const std::string& message) {
        if (!std::filesystem::exists(m_auditFilePath)) {
            return false;
        }
        
        std::ifstream logFile(m_auditFilePath);
        std::string content((std::istreambuf_iterator<char>(logFile)),
                           std::istreambuf_iterator<char>());
        
        return content.find(message) != std::string::npos;
    }
    
    // 辅助函数：获取文件中的行数
    int getFileLineCount() {
        if (!std::filesystem::exists(m_auditFilePath)) {
            return 0;
        }
        
        std::ifstream logFile(m_auditFilePath);
        int lineCount = 0;
        std::string line;
        
        while (std::getline(logFile, line)) {
            lineCount++;
        }
        
        return lineCount;
    }
    
    std::string m_auditFilePath;
};

// 测试基本审计功能
TEST_F(AuditTest, BasicAudit) {
    // 记录一条审计信息
    AUDIT_LOG("testuser", "127.0.0.1", AuditActionType::AUTHENTICATION, 
              "Login Attempt", AuditResult::SUCCESS, "Login successful");
    
    // 验证消息已记录
    EXPECT_TRUE(logContainsMessage("testuser"));
    EXPECT_TRUE(logContainsMessage("127.0.0.1"));
    EXPECT_TRUE(logContainsMessage("AUTHENTICATION"));
    EXPECT_TRUE(logContainsMessage("Login Attempt"));
    EXPECT_TRUE(logContainsMessage("SUCCESS"));
    EXPECT_TRUE(logContainsMessage("Login successful"));
}

// 测试不同类型的审计
TEST_F(AuditTest, DifferentAuditTypes) {
    // 记录几种不同类型的审计
    AUDIT_LOG("admin", "192.168.1.1", AuditActionType::ADMIN_ACTION,
              "Server Restart", AuditResult::SUCCESS, "Scheduled restart");
              
    AUDIT_LOG("user1", "10.0.0.1", AuditActionType::ACCOUNT_MANAGEMENT,
              "Create Account", AuditResult::FAILURE, "Username already exists");
              
    AUDIT_LOG("user2", "10.0.0.2", AuditActionType::CHARACTER_MANAGEMENT,
              "Create Character", AuditResult::SUCCESS, "New character created");
    
    // 验证每种类型的消息都被记录
    EXPECT_TRUE(logContainsMessage("ADMIN_ACTION"));
    EXPECT_TRUE(logContainsMessage("Server Restart"));
    
    EXPECT_TRUE(logContainsMessage("ACCOUNT_MANAGEMENT"));
    EXPECT_TRUE(logContainsMessage("Create Account"));
    EXPECT_TRUE(logContainsMessage("FAILURE"));
    
    EXPECT_TRUE(logContainsMessage("CHARACTER_MANAGEMENT"));
    EXPECT_TRUE(logContainsMessage("Create Character"));
}

// 测试获取最近条目
TEST_F(AuditTest, GetRecentEntries) {
    // 记录10条审计信息
    for (int i = 0; i < 10; i++) {
        AUDIT_LOG("user" + std::to_string(i), "127.0.0." + std::to_string(i),
                 AuditActionType::AUTHENTICATION, "Login " + std::to_string(i),
                 AuditResult::SUCCESS, "Test entry " + std::to_string(i));
    }
    
    // 获取最近5条记录
    auto entries = AuditLog::getInstance().getRecentEntries(5);
    
    // 验证返回了5条
    EXPECT_EQ(entries.size(), 5);
    
    // 验证这是最新的5条（序号5-9）
    bool found5to9 = true;
    for (int i = 5; i < 10; i++) {
        bool foundThis = false;
        for (const auto& entry : entries) {
            if (entry.actionDescription == "Login " + std::to_string(i)) {
                foundThis = true;
                break;
            }
        }
        if (!foundThis) {
            found5to9 = false;
            break;
        }
    }
    EXPECT_TRUE(found5to9);
}

// 测试过滤条目
TEST_F(AuditTest, FilterEntries) {
    // 记录不同类型的审计信息
    AUDIT_LOG("admin", "127.0.0.1", AuditActionType::ADMIN_ACTION,
             "Admin Operation 1", AuditResult::SUCCESS, "Details 1");
             
    AUDIT_LOG("admin", "127.0.0.1", AuditActionType::ADMIN_ACTION,
             "Admin Operation 2", AuditResult::FAILURE, "Details 2");
             
    AUDIT_LOG("user1", "127.0.0.2", AuditActionType::AUTHENTICATION,
             "User Operation", AuditResult::SUCCESS, "Details 3");
    
    // 按操作类型过滤
    auto adminEntries = AuditLog::getInstance().getEntriesByFilter(
        [](const AuditEntry& entry) { 
            return entry.actionType == AuditActionType::ADMIN_ACTION; 
        }
    );
    
    // 验证结果
    EXPECT_EQ(adminEntries.size(), 2);
    for (const auto& entry : adminEntries) {
        EXPECT_EQ(entry.actionType, AuditActionType::ADMIN_ACTION);
    }
    
    // 按结果过滤
    auto successEntries = AuditLog::getInstance().getEntriesByFilter(
        [](const AuditEntry& entry) { 
            return entry.result == AuditResult::SUCCESS; 
        }
    );
    
    // 验证结果
    EXPECT_EQ(successEntries.size(), 2);
    for (const auto& entry : successEntries) {
        EXPECT_EQ(entry.result, AuditResult::SUCCESS);
    }
}

// 测试启用/禁用功能
TEST_F(AuditTest, EnableDisable) {
    // 默认是启用的，记录一条消息
    AUDIT_LOG("user1", "127.0.0.1", AuditActionType::AUTHENTICATION,
             "First Login", AuditResult::SUCCESS, "Enabled test");
    
    // 计算当前行数
    int linesWhenEnabled = getFileLineCount();
    
    // 禁用审计日志
    AuditLog::getInstance().setEnabled(false);
    EXPECT_FALSE(AuditLog::getInstance().isEnabled());
    
    // 尝试记录消息
    AUDIT_LOG("user1", "127.0.0.1", AuditActionType::AUTHENTICATION,
             "Second Login", AuditResult::SUCCESS, "Disabled test");
    
    // 行数应该没有变化
    EXPECT_EQ(getFileLineCount(), linesWhenEnabled);
    
    // 重新启用
    AuditLog::getInstance().setEnabled(true);
    EXPECT_TRUE(AuditLog::getInstance().isEnabled());
    
    // 再次记录消息
    AUDIT_LOG("user1", "127.0.0.1", AuditActionType::AUTHENTICATION,
             "Third Login", AuditResult::SUCCESS, "Re-enabled test");
    
    // 行数应该增加
    EXPECT_GT(getFileLineCount(), linesWhenEnabled);
}

// 测试回调函数
TEST_F(AuditTest, CallbackFunction) {
    // 创建一个标志和一个测试回调
    bool callbackCalled = false;
    AuditEntry callbackEntry;
    
    auto callback = [&callbackCalled, &callbackEntry](const AuditEntry& entry) {
        callbackCalled = true;
        callbackEntry = entry;
    };
    
    // 设置回调
    AuditLog::getInstance().setAuditCallback(callback);
    
    // 记录一条消息
    AUDIT_LOG("callback_user", "127.0.0.1", AuditActionType::SECURITY,
             "Security Event", AuditResult::WARNING, "Callback test");
    
    // 验证回调被调用
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackEntry.userId, "callback_user");
    EXPECT_EQ(callbackEntry.actionType, AuditActionType::SECURITY);
    EXPECT_EQ(callbackEntry.result, AuditResult::WARNING);
}

// 测试并发审计
TEST_F(AuditTest, ConcurrentAudit) {
    const int numThreads = 5;
    const int entriesPerThread = 10;
    
    // 创建多个线程同时写入审计日志
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++) {
        threads.push_back(std::thread([i, entriesPerThread]() {
            for (int j = 0; j < entriesPerThread; j++) {
                AUDIT_LOG("thread" + std::to_string(i), "127.0.0." + std::to_string(i),
                         AuditActionType::CUSTOM, "Thread operation " + std::to_string(j),
                         AuditResult::SUCCESS, "Concurrent test from thread " + std::to_string(i));
                
                // 小延迟，增加并发的可能性
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }));
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有条目都被记录
    auto entries = AuditLog::getInstance().getRecentEntries(numThreads * entriesPerThread);
    
    // 预期条目数（考虑额外的系统条目）
    int expectedEntries = numThreads * entriesPerThread;
    EXPECT_GE(entries.size(), expectedEntries);
    
    // 验证每个线程的条目都存在
    for (int i = 0; i < numThreads; i++) {
        std::string threadId = "thread" + std::to_string(i);
        int entriesForThisThread = 0;
        
        for (const auto& entry : entries) {
            if (entry.userId == threadId) {
                entriesForThisThread++;
            }
        }
        
        EXPECT_EQ(entriesForThisThread, entriesPerThread);
    }
}

// 测试CSV导出
TEST_F(AuditTest, ExportToCSV) {
    // 记录几条审计信息
    AUDIT_LOG("export_user", "127.0.0.1", AuditActionType::DATABASE,
             "Database Query", AuditResult::SUCCESS, "Export test entry 1");
             
    AUDIT_LOG("export_user", "127.0.0.1", AuditActionType::DATABASE,
             "Database Update", AuditResult::SUCCESS, "Export test entry 2");
    
    // 导出到CSV
    std::string csvPath = "test_export.csv";
    bool exportResult = AuditLog::getInstance().exportToCSV(csvPath);
    
    // 验证导出成功
    EXPECT_TRUE(exportResult);
    EXPECT_TRUE(std::filesystem::exists(csvPath));
    
    // 验证CSV内容
    std::ifstream csvFile(csvPath);
    std::string content((std::istreambuf_iterator<char>(csvFile)),
                       std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("export_user"), std::string::npos);
    EXPECT_NE(content.find("DATABASE"), std::string::npos);
    EXPECT_NE(content.find("Database Query"), std::string::npos);
    EXPECT_NE(content.find("Database Update"), std::string::npos);
    
    // 清理导出文件
    if (std::filesystem::exists(csvPath)) {
        std::filesystem::remove(csvPath);
    }
}

} // namespace test
} // namespace utils
} // namespace d3server
```

```markdown
# D3Server 调试指南

本文档提供有关 D3Server 调试功能的详细信息，包括日志系统、调试工具、审计日志和性能监控，帮助开发者和管理员诊断问题和优化服务器性能。

## 目录

- [日志系统](#日志系统)
- [调试模式](#调试模式)
- [审计日志](#审计日志)
- [性能监控](#性能监控)
- [错误排查](#错误排查)
- [调试命令](#调试命令)
- [常见问题](#常见问题)

## 日志系统

D3Server 使用多级日志系统记录运行信息，帮助识别和解决问题。

### 日志级别

- **DEBUG**: 详细的调试信息，包括函数调用、变量值等
- **INFO**: 一般操作信息，如服务器启动、新连接等
- **WARNING**: 潜在问题警告，不影响系统正常运行
- **ERROR**: 错误信息，表明功能未正确执行
- **CRITICAL**: 严重错误，可能导致服务崩溃

### 日志文件位置

默认情况下，日志文件存储在以下位置：

- 主日志: `logs/d3server.log`
- 审计日志: `logs/audit.log`
- 错误日志: `logs/error.log`

### 日志配置

可以在 `config.ini` 文件中配置日志系统：

```ini
[Logging]
LogLevel = INFO           # 日志级别: DEBUG, INFO, WARNING, ERROR, CRITICAL
LogToConsole = true       # 是否输出到控制台
LogFilePath = logs/d3server.log  # 日志文件路径
MaxLogFileSize = 10       # 单个日志文件最大大小(MB)
MaxLogFiles = 5           # 保留的日志文件数量
```

### 日志格式

日志条目格式如下：

```
2023-09-15 14:30:45.123 [INFO] Server started on port 8080
2023-09-15 14:31:12.456 [ERROR] Failed to connect to database: Connection refused
```

## 调试模式

调试模式提供更详细的日志和性能信息，适用于开发和故障排除。

### 启用调试模式

可以通过以下方式启用调试模式：

1. **启动参数**:
   ```bash
   ./d3server --debug
   ```

2. **配置文件**:
   ```ini
   [Server]
   DebugMode = true
   ```

3. **REST API**:
   ```bash
   curl -X POST "http://localhost:8080/api/admin/debug/enable" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

### 调试宏

代码中的调试宏可以在定义了 `DEBUG_MODE` 时自动激活：

```cpp
DEBUG_LOG("消息内容");               // 记录一般调试消息
DEBUG_FUNCTION_ENTER();             // 记录函数进入
DEBUG_FUNCTION_EXIT();              // 记录函数退出
DEBUG_VARIABLE(variableName);       // 记录变量值
DEBUG_TIMER_START("操作名称");       // 开始计时操作
DEBUG_TIMER_STOP();                 // 停止计时并记录耗时
```

调试日志输出示例：

```
2023-09-15 14:35:22.123 [DEBUG] server.cpp:45 (Server::start) Function entered
2023-09-15 14:35:22.124 [DEBUG] server.cpp:50 (Server::start) port = 8080
2023-09-15 14:35:22.150 [DEBUG] server.cpp:120 (Server::start) Function exited
```

## 审计日志

审计日志系统记录关键操作和安全相关事件，用于故障排查和安全审计。

### 审计日志内容

审计日志记录以下类型的操作：

- **认证操作**: 登录尝试、登出、密码重置
- **账号管理**: 账号创建、修改、删除
- **角色管理**: 角色创建、属性修改、删除
- **游戏会话**: 会话创建、玩家加入/离开
- **配置更改**: 服务器设置修改
- **管理员操作**: 特权命令执行
- **安全事件**: 异常登录尝试、访问控制违规

### 审计日志格式

审计日志使用CSV格式，包含以下字段：

```
时间戳,用户ID,IP地址,操作类型,操作描述,结果,详细信息,源代码位置
```

示例：
```
2023-09-15 14:32:45.789,"admin","192.168.1.100","ADMIN_ACTION","Server Restart","SUCCESS","Scheduled maintenance","admin_controller.cpp:156 AdminController::restartServer"
```

### 审计日志配置

可以在配置文件中设置审计日志：

```ini
[Audit]
Enabled = true                  # 是否启用审计日志
LogFilePath = logs/audit.log    # 审计日志文件路径
MaxEntries = 10000              # 内存中保留的最大条目数
```

### 查看审计日志

1. **通过REST API查看**:
   ```bash
   curl -X GET "http://localhost:8080/api/admin/logs/audit?type=AUTHENTICATION&limit=100" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

2. **导出为CSV**:
   ```bash
   curl -X GET "http://localhost:8080/api/admin/logs/audit/export" \
     -H "Authorization: Bearer YOUR_API_KEY" \
     -o audit_export.csv
   ```

3. **过滤审计日志**:
   
   可以按用户、操作类型、结果类型进行过滤：
   ```bash
   curl -X GET "http://localhost:8080/api/admin/logs/audit?user=admin&result=FAILURE" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

## 性能监控

D3Server 包含多种性能监控工具，帮助识别瓶颈和优化性能。

### 性能指标

系统跟踪以下性能指标：

- **CPU利用率**: 整体和每个线程的CPU使用情况
- **内存使用**: 堆内存、栈内存、系统内存使用情况
- **网络流量**: 传入/传出流量，连接数量
- **数据库性能**: 查询时间、连接数、缓存命中率
- **请求延迟**: 不同操作的平均响应时间
- **并发度**: 同时在线玩家数、活动会话数

### 查看性能统计

1. **通过REST API查看**:
   ```bash
   curl -X GET "http://localhost:8080/api/admin/stats" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

2. **实时监控**:
   管理界面提供实时性能监控页面，访问：
   ```
   http://localhost:8080/admin/dashboard
   ```

3. **性能日志**:
   在调试模式下，性能指标会定期写入日志文件:
   ```
   logs/performance.log
   ```

### 性能分析

1. **函数计时**:
   
   使用调试计时器分析函数性能：
   ```cpp
   DEBUG_TIMER_START("DatabaseQuery");
   // 执行查询...
   DEBUG_TIMER_STOP(); // 输出: "Operation 'DatabaseQuery' completed in XXX μs"
   ```

2. **内存分析**:
   
   使用 Valgrind 或其他内存分析工具：
   ```bash
   valgrind --tool=massif ./d3server
   ```

## 错误排查

### 常见错误代码

| 错误代码 | 描述 | 可能原因 | 解决方案 |
|---------|------|---------|---------|
| ERR_DB_001 | 数据库连接失败 | SQLite文件权限问题或文件损坏 | 检查文件权限和完整性 |
| ERR_NET_001 | 端口绑定失败 | 端口已被占用 | 更改配置中的端口或释放该端口 |
| ERR_AUTH_001 | 认证失败 | 密码错误或账号不存在 | 检查账号状态和密码 |
| ERR_MEM_001 | 内存分配失败 | 系统内存不足 | 增加系统内存或减少服务器负载 |

### 调试工具

1. **内存泄漏检测**:
   ```bash
   valgrind --leak-check=full ./d3server
   ```

2. **线程分析**:
   ```bash
   valgrind --tool=helgrind ./d3server
   ```

3. **核心转储分析**:
   ```bash
   gdb ./d3server core.XXXX
   ```

## 调试命令

D3Server 支持多种调试命令，方便开发和管理。

### REST API 调试命令

| 命令 | 描述 | 示例 |
|-----|------|-----|
| `/api/admin/debug/threads` | 显示线程信息 | `curl -X GET "http://localhost:8080/api/admin/debug/threads"` |
| `/api/admin/debug/memory` | 显示内存使用情况 | `curl -X GET "http://localhost:8080/api/admin/debug/memory"` |
| `/api/admin/debug/connections` | 显示当前连接 | `curl -X GET "http://localhost:8080/api/admin/debug/connections"` |
| `/api/admin/debug/gc` | 触发垃圾收集 | `curl -X POST "http://localhost:8080/api/admin/debug/gc"` |

### 控制台命令

在服务器控制台中可使用以下命令：

```
debug enable              # 启用调试模式
debug disable             # 禁用调试模式
debug memory              # 显示内存使用
debug threads             # 显示线程信息
debug log <level>         # 设置日志级别
debug reload-config       # 重新加载配置
audit enable              # 启用审计
audit disable             # 禁用审计
```

## 常见问题

### 日志文件持续增长

**问题**: 日志文件不断增长，占用大量磁盘空间。

**解决方案**: 启用日志轮转功能：

```ini
[Logging]
EnableRotation = true
MaxLogFileSize = 10       # MB
MaxLogFiles = 5
```

### 性能下降

**问题**: 服务器随时间性能下降。

**排查步骤**:
1. 检查内存泄漏: `valgrind --leak-check=full ./d3server`
2. 查看性能日志: `logs/performance.log`
3. 检查数据库索引
4. 分析连接数和网络流量

### 服务器崩溃

**问题**: 服务器意外崩溃。

**排查步骤**:
1. 检查核心转储: `gdb ./d3server core.XXXX`
2. 查看错误日志: `logs/error.log`
3. 启用更详细的调试信息: `./d3server --debug`
4. 检查系统资源使用情况: `top`, `htop`, `iotop`

### 无法连接数据库

**问题**: 服务器无法连接到数据库。

**解决方案**:
1. 检查数据库文件权限
2. 确保数据库目录存在并可写
3. 验证数据库未损坏: `sqlite3 <db-file> "PRAGMA integrity_check;"`

---

如需更多帮助，请提交 GitHub issue 或联系项目维护人员。
```

```cpp
#include "battle_net/battle_net_client.h"
#include "battle_net/battle_net_server.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/crypto_utils.h"
#include "utils/audit.h"
#include "database/database_manager.h"

#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>

namespace d3server {
namespace battle_net {

// Header size for Battle.net messages
constexpr size_t HEADER_SIZE = 4;

BattleNetClient::BattleNetClient(
    boost::asio::ip::tcp::socket socket,
    BattleNetServer& server,
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_socket(std::move(socket)),
    m_server(server),
    m_config(config),
    m_dbManager(dbManager),
    m_connected(true),
    m_authState(AuthState::NotAuthenticated),
    m_headerBuffer(HEADER_SIZE),
    m_writing(false)
{
    DEBUG_FUNCTION_ENTER();
    
    // Get client IP address
    try {
        m_ipAddress = m_socket.remote_endpoint().address().to_string();
    }
    catch (const std::exception& e) {
        m_ipAddress = "unknown";
        DEBUG_LOG("Failed to get client IP address: " + std::string(e.what()));
    }
    
    // Set client connect time
    m_connectTime = std::time(nullptr);
    m_lastActivityTime = m_connectTime;
    
    LOG_INFO("Battle.net client created from " + m_ipAddress);
    
    // 记录审计日志：客户端连接
    AUDIT_LOG("SYSTEM", m_ipAddress, utils::AuditActionType::NETWORK,
              "Client Connection", utils::AuditResult::SUCCESS,
              "Battle.net client connected");
              
    DEBUG_FUNCTION_EXIT();
}

BattleNetClient::~BattleNetClient() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_connected) {
        disconnect();
    }
    
    LOG_INFO("Battle.net client destroyed from " + m_ipAddress);
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::start() {
    DEBUG_FUNCTION_ENTER();
    
    // Start reading messages
    readHeader();
    
    // Send welcome message
    std::vector<uint8_t> welcomeMsg = { 0x01, 0x00, 0x00, 0x00, 0x00 };
    sendMessage(welcomeMsg);
    
    DEBUG_FUNCTION_EXIT();
}

// ... 省略其他未修改的函数 ...

void BattleNetClient::disconnect() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_connected = false;
    
    try {
        if (m_socket.is_open()) {
            boost::system::error_code ec;
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during client disconnect: " + std::string(e.what()));
    }
    
    LOG_INFO("Client disconnected: " + m_ipAddress);
    
    // 记录审计日志：客户端断开连接
    AUDIT_LOG(m_login.empty() ? "UNKNOWN" : m_login, m_ipAddress, 
              utils::AuditActionType::NETWORK, "Client Disconnection",
              utils::AuditResult::SUCCESS, "Battle.net client disconnected");
              
    DEBUG_FUNCTION_EXIT();
}

bool BattleNetClient::handleAuth(const std::string& login, const std::string& password) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            // 记录审计日志：账号不存在
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Attempt", utils::AuditResult::FAILURE,
                     "Account does not exist");
                     
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Check if account is banned
        if (m_dbManager->isAccountBanned(login)) {
            LOG_WARNING("Login attempt on banned account: " + login + " from " + m_ipAddress);
            
            // 记录审计日志：尝试登录被封禁账号
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Attempt", utils::AuditResult::UNAUTHORIZED,
                     "Account is banned");
                     
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Verify password
        bool success = m_dbManager->verifyAccountPassword(login, password);
        
        if (success) {
            // Update last login time
            m_dbManager->updateAccountLastLogin(login);
            
            // 记录审计日志：登录成功
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Success", utils::AuditResult::SUCCESS,
                     "Authentication successful");
        } else {
            // 记录审计日志：密码错误
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Failure", utils::AuditResult::FAILURE,
                     "Invalid password");
        }
        
        DEBUG_FUNCTION_EXIT();
        return success;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during authentication: " + std::string(e.what()));
        
        // 记录审计日志：认证过程发生异常
        AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                 "Login Error", utils::AuditResult::FAILURE,
                 "Exception: " + std::string(e.what()));
                 
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void BattleNetClient::setAuthState(AuthState state) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(static_cast<int>(state));
    
    AuthState oldState = m_authState;
    m_authState = state;
    
    // 记录审计日志：认证状态变更
    if (oldState != state) {
        std::string stateStr;
        switch (state) {
            case AuthState::NotAuthenticated: stateStr = "NotAuthenticated"; break;
            case AuthState::InProgress: stateStr = "InProgress"; break;
            case AuthState::Authenticated: stateStr = "Authenticated"; break;
            case AuthState::AuthFailed: stateStr = "AuthFailed"; break;
            default: stateStr = "Unknown"; break;
        }
        
        utils::AuditResult auditResult;
        switch (state) {
            case AuthState::Authenticated: 
                auditResult = utils::AuditResult::SUCCESS; 
                break;
            case AuthState::AuthFailed: 
                auditResult = utils::AuditResult::FAILURE; 
                break;
            default: 
                auditResult = utils::AuditResult::WARNING; 
                break;
        }
        
        AUDIT_LOG(m_login.empty() ? "UNKNOWN" : m_login, m_ipAddress,
                 utils::AuditActionType::AUTHENTICATION, "Auth State Changed",
                 auditResult, "New state: " + stateStr);
    }
    
    DEBUG_FUNCTION_EXIT();
}

// ... 省略其他未修改的函数 ...

// 处理认证相关消息的函数
void BattleNetClient::handleAuthMessage(const std::vector<uint8_t>& message) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        if (message.size() < 2) {
            LOG_WARNING("Auth message too short from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG("UNKNOWN", m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Message too short");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Extract login length
        uint8_t loginLength = message[1];
        if (message.size() < 2 + loginLength) {
            LOG_WARNING("Auth message missing login from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG("UNKNOWN", m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Missing login data");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Extract login
        std::string login(message.begin() + 2, message.begin() + 2 + loginLength);
        
        // Extract password length
        if (message.size() < 2 + loginLength + 1) {
            LOG_WARNING("Auth message missing password length from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Missing password length");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        uint8_t passwordLength = message[2 + loginLength];
        if (message.size() < 2 + loginLength + 1 + passwordLength) {
            LOG_WARNING("Auth message missing password from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Missing password data");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Extract password
        std::string password(message.begin() + 2 + loginLength + 1, 
                            message.begin() + 2 + loginLength + 1 + passwordLength);
        
        // Set state to in progress
        setAuthState(AuthState::InProgress);
        
        // 记录审计日志：开始认证过程
        AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                 "Authentication Started", utils::AuditResult::WARNING,
                 "Authentication in progress");
        
        // Authenticate
        bool authSuccess = handleAuth(login, password);
        
        // Set login and auth state
        if (authSuccess) {
            m_login = login;
            setAuthState(AuthState::Authenticated);
            
            // 记录获取角色操作
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::CHARACTER_MANAGEMENT,
                     "Characters Requested", utils::AuditResult::SUCCESS,
                     "Character list requested after successful login");
        } else {
            m_login.clear();
            setAuthState(AuthState::AuthFailed);
        }
        
        // Send response
        std::vector<uint8_t> response = { 0x02, authSuccess ? (uint8_t)0x01 : (uint8_t)0x00 };
        sendMessage(response);
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception handling auth message: " + std::string(e.what()));
        
        // 记录审计日志：处理认证消息时发生异常
        AUDIT_LOG("UNKNOWN", m_ipAddress, utils::AuditActionType::SECURITY,
                 "Auth Message Error", utils::AuditResult::FAILURE,
                 "Exception: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

// ... 省略其他未修改的函数 ...

} // namespace battle_net
} // namespace d3server
```

```cpp
#pragma once

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <ctime>
#include <memory>

namespace d3server {

namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace battle_net {

class BattleNetServer;

/**
 * @brief Authentication state of the client
 */
enum class AuthState {
    NotAuthenticated, ///< Client is not authenticated
    InProgress,       ///< Authentication is in progress
    Authenticated,    ///< Client is authenticated
    AuthFailed        ///< Authentication failed
};

/**
 * @brief Battle.net client class
 * 
 * This class represents a client connection to the Battle.net server
 */
class BattleNetClient : public std::enable_shared_from_this<BattleNetClient> {
public:
    /**
     * @brief Constructor
     * @param socket TCP socket
     * @param server Reference to the server
     * @param config Server configuration
     * @param dbManager Database manager
     */
    BattleNetClient(
        boost::asio::ip::tcp::socket socket,
        BattleNetServer& server,
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~BattleNetClient();
    
    /**
     * @brief Start reading from the socket
     */
    void start();
    
    /**
     * @brief Send a message to the client
     * @param message Message to send
     */
    virtual void sendMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Get client's login
     * @return Client's login
     */
    std::string getLogin() const { return m_login; }
    
    /**
     * @brief Get client's IP address
     * @return Client's IP address
     */
    std::string getIpAddress() const { return m_ipAddress; }
    
    /**
     * @brief Get client's authentication state
     * @return Authentication state
     */
    AuthState getAuthState() const { return m_authState; }
    
    /**
     * @brief Set client's authentication state
     * @param state New authentication state
     */
    void setAuthState(AuthState state);
    
    /**
     * @brief Get client's connection time
     * @return Connection time as time_t
     */
    std::time_t getConnectTime() const { return m_connectTime; }
    
    /**
     * @brief Get client's last activity time
     * @return Last activity time as time_t
     */
    std::time_t getLastActivityTime() const { return m_lastActivityTime; }
    
    /**
     * @brief Update client's last activity time to current time
     */
    void updateLastActivityTime();
    
    /**
     * @brief Disconnect the client
     */
    void disconnect();
    
    /**
     * @brief Check if the client is connected
     * @return True if connected, false otherwise
     */
    bool isConnected() const { return m_connected; }
    
protected:
    /**
     * @brief Read message header from the socket
     */
    void readHeader();
    
    /**
     * @brief Read message body from the socket
     * @param bodySize Size of the message body
     */
    void readBody(size_t bodySize);
    
    /**
     * @brief Process received message
     * @param message Received message
     */
    void processMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Handle authentication
     * @param login Client's login
     * @param password Client's password
     * @return True if authentication succeeded, false otherwise
     */
    bool handleAuth(const std::string& login, const std::string& password);
    
    /**
     * @brief Handle authentication message
     * @param message Authentication message
     */
    void handleAuthMessage(const std::vector<uint8_t>& message);
    
    boost::asio::ip::tcp::socket m_socket;
    BattleNetServer& m_server;
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    bool m_connected;
    std::string m_login;
    std::string m_ipAddress;
    AuthState m_authState;
    std::time_t m_connectTime;
    std::time_t m_lastActivityTime;
    std::vector<uint8_t> m_headerBuffer;
    std::vector<uint8_t> m_messageBuffer;
    bool m_writing;
    std::vector<std::vector<uint8_t>> m_messageQueue;
};

} // namespace battle_net
} // namespace d3server
```

基于你的要求，我已经完成了对代码和文档的增强，主要添加了以下内容：

1. **调试审计系统**：
   - 创建了新的审计日志系统（`utils/audit.h` 和 `utils/audit.cpp`），可记录关键操作如用户登录、账户管理、网络连接等
   - 在 Battle.net 客户端代码中添加了审计记录，跟踪认证状态变化和安全事件
   - 审计日志支持CSV导出、过滤和查询功能

2. **中文文档**：
   - 创建了 `CONTRIBUTING_CN.md` 中文贡献指南，详细说明代码规范、提交流程和测试要求
   - 添加了 `debugging_cn.md` 中文调试指南，包括调试工具使用和问题排查流程

3. **测试完善**：
   - 添加了审计日志系统的全面测试（`audit_tests.cpp`），测试基本功能、并发处理和回调机制
   - 测试覆盖了不同类型的审计记录和过滤功能

这些增强使得：

1. 当出现问题时，审计日志可以帮助快速定位问题原因，包括安全相关问题和用户操作错误
2. 完善的中文文档让其他开发者更容易参与项目
3. 测试用例可以确保各组件正常工作，防止回归问题

最值得注意的是，新的审计系统完全独立于现有的日志系统，提供了更专注于安全和操作审计的功能，可以在需要时导出为CSV进行分析或提供给安全团队。

审计日志记录了关键操作，例如：
- 用户登录尝试（成功/失败）
- 账户管理操作
- 网络连接/断开
- 安全相关事件（如异常登录尝试）

这些改进大大提高了系统的可调试性和安全性，同时也为新开发者提供了更友好的中文文档支持。

