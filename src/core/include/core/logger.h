#ifndef D3SERVER_CORE_LOGGER_H
#define D3SERVER_CORE_LOGGER_H

// 包含 spdlog 的核心头文件和常用的 sinks
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" // 用于控制台输出，带颜色
#include "spdlog/sinks/basic_file_sink.h"    // 用于基本文件输出
#include <memory>
#include <string>

namespace d3server {
namespace core {

class Logger {
public:
    /**
     * @brief 初始化日志系统
     * 
     * @param loggerName 日志记录器的名称，将用于日志文件和控制台输出的前缀。
     * @param logFilePath 日志文件的路径。如果为空字符串，则不创建文件日志。
     * @param logLevel 最低的日志级别。
     * @param consoleOutput 是否输出到控制台。
     * @param fileOutput 是否输出到文件。
     */
    static void init(
        const std::string& loggerName = "d3server", 
        const std::string& logFilePath = "logs/d3server.log", 
        spdlog::level::level_enum logLevel = spdlog::level::debug, 
        bool consoleOutput = true,
        bool fileOutput = true
    );

    /**
     * @brief 获取已配置的 spdlog 日志记录器实例。
     * 
     * @return std::shared_ptr<spdlog::logger> 指向 spdlog 日志记录器的共享指针。
     *         如果日志系统未初始化，则可能返回 nullptr 或抛出异常（取决于实现）。
     *         建议在使用前总是先调用 init()。
     */
    static std::shared_ptr<spdlog::logger> getLogger();

private:
    static std::shared_ptr<spdlog::logger> s_logger;
    static bool s_initialized;
};

// 基于 spdlog 的便捷日志宏
// 这些宏将使用 Logger::getLogger() 获取的记录器实例

// 注意：在多线程环境中直接使用全局静态logger实例（如 s_logger）需要谨慎处理其初始化。
// 一个更健壮的方法是让 getLogger() 总是返回一个有效的实例，并在首次调用时进行初始化。
// 或者，在应用程序启动时显式调用 Logger::init()。

#define LOG_SPD_TRACE(...)    if(d3server::core::Logger::getLogger()) { d3server::core::Logger::getLogger()->trace(__VA_ARGS__); } else { /* 处理未初始化的情况 */ }
#define LOG_SPD_DEBUG(...)    if(d3server::core::Logger::getLogger()) { d3server::core::Logger::getLogger()->debug(__VA_ARGS__); } else { /* 处理未初始化的情况 */ }
#define LOG_SPD_INFO(...)     if(d3server::core::Logger::getLogger()) { d3server::core::Logger::getLogger()->info(__VA_ARGS__); } else { /* 处理未初始化的情况 */ }
#define LOG_SPD_WARN(...)     if(d3server::core::Logger::getLogger()) { d3server::core::Logger::getLogger()->warn(__VA_ARGS__); } else { /* 处理未初始化的情况 */ }
#define LOG_SPD_ERROR(...)    if(d3server::core::Logger::getLogger()) { d3server::core::Logger::getLogger()->error(__VA_ARGS__); } else { /* 处理未初始化的情况 */ }
#define LOG_SPD_CRITICAL(...) if(d3server::core::Logger::getLogger()) { d3server::core::Logger::getLogger()->critical(__VA_ARGS__); } else { /* 处理未初始化的情况 */ }

// 沿用旧的宏名，但指向新的 spdlog 实现
#define LOG_DEBUG(...) LOG_SPD_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)  LOG_SPD_INFO(__VA_ARGS__)
#define LOG_WARNING(...) LOG_SPD_WARN(__VA_ARGS__)
#define LOG_ERROR(...) LOG_SPD_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) LOG_SPD_CRITICAL(__VA_ARGS__)

} // namespace core
} // namespace d3server

#endif // D3SERVER_CORE_LOGGER_H 
