#include "core/logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace d3server {
namespace d3core {

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

} // namespace d3core
} // namespace d3server 
