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