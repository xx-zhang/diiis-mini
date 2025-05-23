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