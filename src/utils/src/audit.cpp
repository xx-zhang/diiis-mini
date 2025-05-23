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