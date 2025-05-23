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