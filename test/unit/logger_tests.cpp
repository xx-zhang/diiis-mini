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