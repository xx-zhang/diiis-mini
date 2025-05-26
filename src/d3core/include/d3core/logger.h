#ifndef D3SERVER_CORE_LOGGER_H
#define D3SERVER_CORE_LOGGER_H

#include <string>
#include <iostream>

namespace d3server {
namespace d3core {

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
     * @brief Log a message with the specified log level
     * @param level Log level
     * @param message Message to log
     */
    static void log(LogLevel level, const std::string& message) {
        switch (level) {
            case LogLevel::DEBUG:
                std::cout << "[DEBUG] " << message << std::endl;
                break;
            case LogLevel::INFO:
                std::cout << "[INFO] " << message << std::endl;
                break;
            case LogLevel::WARNING:
                std::cerr << "[WARNING] " << message << std::endl;
                break;
            case LogLevel::ERROR:
                std::cerr << "[ERROR] " << message << std::endl;
                break;
            case LogLevel::CRITICAL:
                std::cerr << "[CRITICAL] " << message << std::endl;
                break;
        }
    }

    // 可以添加更多日志方法，如写入文件等
};

// Convenience macros for logging
#define LOG_DEBUG(message) \
    d3server::core::Logger::log(d3server::core::LogLevel::DEBUG, message)

#define LOG_INFO(message) \
    d3server::core::Logger::log(d3server::core::LogLevel::INFO, message)

#define LOG_WARNING(message) \
    d3server::core::Logger::log(d3server::core::LogLevel::WARNING, message)

#define LOG_ERROR(message) \
    d3server::core::Logger::log(d3server::core::LogLevel::ERROR, message)

#define LOG_CRITICAL(message) \
    d3server::core::Logger::log(d3server::core::LogLevel::CRITICAL, message)

} // namespace d3core
} // namespace d3server

#endif // D3SERVER_CORE_LOGGER_H 
