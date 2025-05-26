#include "core/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h" // 包含滚动文件sink
#include <vector>
#include <iostream> // 用于在未初始化时输出错误

namespace d3server {
namespace core {

// 定义静态成员变量
std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
bool Logger::s_initialized = false;

void Logger::init(
    const std::string& loggerName,
    const std::string& logFilePath,
    spdlog::level::level_enum logLevel,
    bool consoleOutput,
    bool fileOutput
) {
    if (s_initialized) {
        // 可以选择在这里警告或重新初始化，或者直接返回
        // LOG_SPD_WARN("Logger is already initialized.");
        return;
    }

    try {
        std::vector<spdlog::sink_ptr> sinks;

        if (consoleOutput) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(logLevel); // 可以为不同的sink设置不同的级别
            // console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"); // 自定义格式
            sinks.push_back(console_sink);
        }

        if (fileOutput && !logFilePath.empty()) {
            // 使用滚动文件日志，例如，最大5MB，保留3个文件
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, 1024 * 1024 * 5, 3);
            file_sink->set_level(logLevel);
            // file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v"); // 自定义格式
            sinks.push_back(file_sink);
        }

        if (sinks.empty()) {
            // 如果没有配置任何 sink，至少创建一个控制台 sink 以避免 spdlog 崩溃
             auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
             console_sink->set_level(spdlog::level::warn); // 默认警告级别
             sinks.push_back(console_sink);
             std::cerr << "[Logger Warning] No sinks configured for logger '" << loggerName << "'. Defaulting to console warning output." << std::endl;
        }

        // 可以选择异步日志以提高性能
        // spdlog::init_thread_pool(8192, 1); 
        // s_logger = std::make_shared<spdlog::async_logger>(
        //     loggerName, 
        //     sinks.begin(), sinks.end(), 
        //     spdlog::thread_pool(), 
        //     spdlog::async_overflow_policy::block
        // );

        // 创建同步日志记录器
        s_logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
        
        s_logger->set_level(logLevel);
        spdlog::register_logger(s_logger); // 注册logger，使其可以通过 spdlog::get() 获取
        // spdlog::set_default_logger(s_logger); // 可选：设置为默认logger
        
        // 设置全局日志格式和级别 (如果需要统一)
        // spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        // spdlog::set_level(logLevel);

        // 刷新策略 (例如，每3秒刷新一次)
        // s_logger->flush_on(spdlog::level::info);
        // spdlog::flush_every(std::chrono::seconds(3));

        s_initialized = true;
        LOG_SPD_INFO("Logger '{} ' initialized. Log level: {}. Console: {}, File: {}", 
                     loggerName, spdlog::level::to_string_view(logLevel), consoleOutput, fileOutput);

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        s_initialized = false;
        // 确保 s_logger 在异常时是 nullptr 或一个安全的备用 logger
        s_logger = nullptr; 
    }
}

std::shared_ptr<spdlog::logger> Logger::getLogger() {
    if (!s_initialized || !s_logger) {
        // 可以选择在这里尝试重新初始化一个最小配置的logger，或者直接返回nullptr
        // 或者抛出异常，强制用户先初始化
        // std::cerr << "[Logger Error] Logger not initialized or initialization failed. Call Logger::init() first." << std::endl;
        // 返回一个临时的、最小配置的控制台logger，以避免空指针调用，但这可能会隐藏初始化问题
        // static auto fallback_logger = spdlog::stdout_color_mt("fallback_logger");
        // fallback_logger->set_level(spdlog::level::err);
        // fallback_logger->error("Attempted to use logger before initialization or after failed initialization!");
        // return fallback_logger;
        return nullptr; // 或者抛出异常
    }
    return s_logger;
}

} // namespace core
} // namespace d3server 
