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