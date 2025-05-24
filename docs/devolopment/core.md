# Core Module (`core`)

 The `core` module forms the backbone of the D3Server application. It contains the main server logic, configuration management, and logging facilities.
 
 ## Core Components
 
 ### `Server` Class (`core/include/core/server.h`)
 
 The `Server` class is the central orchestrator of the D3Server. It initializes, runs, and shuts down all other major server components like the Battle.Net server, Game server, and REST API server.
 
 #### Public Methods
 
 *   `Server(Config& config, Debug& debug, Logger& logger)`: Constructor. Initializes the server with the given configuration, debug, and logger instances.
 *   `~Server()`: Destructor. Responsible for ensuring all components are properly shut down.
 *   `bool initialize()`: Initializes all server components. This includes setting up the database connection (via the `Database` module), initializing network listeners for Battle.Net, Game, and REST API servers. Returns `true` on success, `false` otherwise.
 *   `void run()`: Starts the main server loop. This typically involves spawning threads for the Battle.Net, Game, and REST API servers to handle client connections and requests concurrently. This method will block until the server is shut down.
 *   `void shutdown()`: Initiates a graceful shutdown of all server components. Signals all component threads to stop, closes network connections, and performs any necessary cleanup.
 *   `void signalHandler(int signum)`: A static or standalone function (often registered with `signal()`) to catch system signals like `SIGINT` and `SIGTERM`. It should call the `shutdown()` method on the global `Server` instance to ensure graceful termination.
 
 ### `Config` Class (`core/include/core/config.h`)
 
 The `Config` class is responsible for loading and providing access to server configuration settings from an INI file.
 
 #### Public Methods
 
 *   `Config()`: Default constructor.
 *   `bool load(const std::string& file_path)`: Loads configuration from the specified INI file. Parses sections like `[Database]`, `[Network]`, `[Server]`. Returns `true` on successful loading and parsing, `false` otherwise (e.g., file not found, parse error).
 *   `const DatabaseConfig& getDatabaseConfig() const`: Returns a constant reference to the `DatabaseConfig` structure, which contains database connection parameters (e.g., file path for SQLite).
 *   `const NetworkConfig& getNetworkConfig(const std::string& server_type) const`: Returns a constant reference to the `NetworkConfig` for a specific server type (e.g., "BattleNet", "GameServer", "RestApi"). `NetworkConfig` typically includes IP address and port.
 *   `const ServerConfig& getServerConfig() const`: Returns a constant reference to the `ServerConfig` structure, which might contain general server settings (e.g., server name, max players (though this might be game-specific)).
 *   `std::string getRawValue(const std::string& section, const std::string& key, const std::string& default_value = "") const`: Retrieves a raw string value from a given section and key, returning a default value if not found.
 
 #### Nested Structures
 
 *   `struct DatabaseConfig`: 
     *   `std::string type`: Type of database (e.g., "sqlite").
     *   `std::string path`: Path to the SQLite database file.
 *   `struct NetworkConfig`: 
     *   `std::string host`: IP address to bind to.
     *   `int port`: Port number to listen on.
     *   `std::string api_key`: (Specifically for RestApi) The API key required for authentication.
 *   `struct ServerConfig`: 
     *   `std::string server_name`: Name of the server.
     *   `std::string log_level`: Default logging level (e.g., "INFO", "DEBUG").
     *   `std::string log_file`: Path to the log file.
 
 ### `Logger` Class (`core/include/core/logger.h`)
 
 The `Logger` class provides a singleton logging facility for the application. It supports different log levels, console output, file output, and custom log handling callbacks.
 
 #### Log Levels (enum `LogLevel`)
 
 *   `DEBUG`: Detailed information, typically of interest only when diagnosing problems.
 *   `INFO`: Confirmation that things are working as expected.
 *   `WARNING`: An indication that something unexpected happened, or indicative of some problem in the near future (e.g., ‘disk space low’). The software is still working as expected.
 *   `ERROR`: Due to a more serious problem, the software has not been able to perform some function.
 *   `CRITICAL`: A serious error, indicating that the program itself may be unable to continue running.
 
 #### Public Methods
 
 *   `static Logger& getInstance()`: Returns a reference to the singleton `Logger` instance.
 *   `void initialize(const std::string& log_file_path, LogLevel level, bool console_logging = true)`: Initializes the logger with the path to the log file, the minimum log level to record, and whether to also log to the console.
 *   `void setLevel(LogLevel level)`: Sets the minimum log level.
 *   `LogLevel getLevel() const`: Gets the current minimum log level.
 *   `void log(LogLevel level, const std::string& message, const char* file, int line, const char* func)`: The core logging method. Records a message if its level is at or above the configured minimum. Includes timestamp, level, message, source file, line number, and function name.
 *   `void registerCallback(LogCallback callback)`: Registers a callback function to be invoked for every log message. `LogCallback` is a `std::function<void(LogLevel, const std::string&)>`.
 *   `void enableConsoleLogging(bool enable)`: Enables or disables console logging.
 *   `void enableFileLogging(bool enable)`: Enables or disables file logging (assuming it was initialized with a file path).
 
 #### Macros (Typical Usage)
 
 While not part of the class, the logger is typically used via macros for convenience to automatically pass file, line, and function information:
 
 ```cpp
 // Example (actual macros would be defined elsewhere, e.g., in logger.h or a common macros header)
 #define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__, __func__)
 #define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg, __FILE__, __LINE__, __func__)
 // ... and so on for WARNING, ERROR, CRITICAL
 ``` 