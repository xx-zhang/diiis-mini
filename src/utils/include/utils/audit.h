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