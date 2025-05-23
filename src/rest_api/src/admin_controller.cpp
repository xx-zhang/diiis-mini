#include "rest_api/admin_controller.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "database/database_manager.h"

#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

namespace d3server {
namespace rest_api {

AdminController::AdminController(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AdminController created");
    DEBUG_FUNCTION_EXIT();
}

AdminController::~AdminController() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AdminController destroyed");
    DEBUG_FUNCTION_EXIT();
}

HttpResponse AdminController::banAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Parse ban data
        json banData = json::parse(jsonStr);
        
        // Extract ban status
        bool banned = banData.contains("banned") ? banData["banned"].get<bool>() : true;
        std::string reason = banData.contains("reason") ? banData["reason"].get<std::string>() : "";
        
        // Ban or unban account
        bool success = m_dbManager->setBanStatus(login, banned, reason);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to update ban status");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        
        if (banned) {
            response.body = R"({"success":true,"message":"Account banned successfully"})";
            LOG_INFO("Account banned: " + login + ", reason: " + reason);
        } else {
            response.body = R"({"success":true,"message":"Account unbanned successfully"})";
            LOG_INFO("Account unbanned: " + login);
        }
        
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception updating ban status: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::getServerStatus(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    try {
        // Get system stats
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream timeStr;
        timeStr << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        
        // Get connected clients/stats from database
        int numAccounts = m_dbManager->getAccountCount();
        int numCharacters = m_dbManager->getTotalCharacterCount();
        int numOnlinePlayers = m_dbManager->getOnlinePlayerCount();
        
        // Create JSON response
        json response = {
            {"server_name", m_config->getServerConfig().serverName},
            {"version", "1.0.0"}, // TODO: Get from version file
            {"uptime", "Unknown"}, // TODO: Calculate uptime
            {"current_time", timeStr.str()},
            {"server_stats", {
                {"accounts", numAccounts},
                {"characters", numCharacters},
                {"online_players", numOnlinePlayers},
                {"max_players", m_config->getServerConfig().maxPlayers}
            }},
            {"system_info", {
                {"os", "Linux"}, // TODO: Get OS info
                {"cpu_usage", 0.0}, // TODO: Get CPU usage
                {"memory_usage", 0.0}, // TODO: Get memory usage
                {"debug_enabled", m_config->getServerConfig().enableDebug}
            }}
        };
        
        // Return response
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = response.dump(4); // Pretty print with 4 spaces indentation
        
        DEBUG_FUNCTION_EXIT();
        return httpResponse;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception getting server status: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::shutdownServer(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Parse shutdown data
        json shutdownData = json::parse(jsonStr);
        
        // Extract delay (optional)
        int delay = shutdownData.contains("delay") ? shutdownData["delay"].get<int>() : 0;
        std::string reason = shutdownData.contains("reason") ? shutdownData["reason"].get<std::string>() : "Server maintenance";
        
        // Log shutdown request
        LOG_INFO("Server shutdown requested. Reason: " + reason + ", Delay: " + std::to_string(delay) + " seconds");
        
        // Start shutdown in a separate thread
        if (delay > 0) {
            std::thread([delay, reason]() {
                LOG_INFO("Server will shut down in " + std::to_string(delay) + " seconds. Reason: " + reason);
                std::this_thread::sleep_for(std::chrono::seconds(delay));
                LOG_INFO("Shutting down now...");
                // TODO: Signal main server to shutdown
                // For now, just exit
                exit(0);
            }).detach();
        } else {
            std::thread([]() {
                LOG_INFO("Shutting down now...");
                // A small delay to allow the response to be sent
                std::this_thread::sleep_for(std::chrono::seconds(1));
                // TODO: Signal main server to shutdown
                // For now, just exit
                exit(0);
            }).detach();
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        
        if (delay > 0) {
            response.body = "{\"success\":true,\"message\":\"Server will shut down in " + std::to_string(delay) + " seconds\"}";
        } else {
            response.body = R"({"success":true,"message":"Server is shutting down"})";
        }
        
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during server shutdown: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::restartServer(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Parse restart data
        json restartData = json::parse(jsonStr);
        
        // Extract delay (optional)
        int delay = restartData.contains("delay") ? restartData["delay"].get<int>() : 0;
        std::string reason = restartData.contains("reason") ? restartData["reason"].get<std::string>() : "Server maintenance";
        
        // Log restart request
        LOG_INFO("Server restart requested. Reason: " + reason + ", Delay: " + std::to_string(delay) + " seconds");
        
        // Start restart in a separate thread
        if (delay > 0) {
            std::thread([delay, reason]() {
                LOG_INFO("Server will restart in " + std::to_string(delay) + " seconds. Reason: " + reason);
                std::this_thread::sleep_for(std::chrono::seconds(delay));
                LOG_INFO("Restarting now...");
                // TODO: Signal main server to restart
                // For now, just exit (a service manager would restart the server)
                exit(0);
            }).detach();
        } else {
            std::thread([]() {
                LOG_INFO("Restarting now...");
                // A small delay to allow the response to be sent
                std::this_thread::sleep_for(std::chrono::seconds(1));
                // TODO: Signal main server to restart
                // For now, just exit (a service manager would restart the server)
                exit(0);
            }).detach();
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        
        if (delay > 0) {
            response.body = "{\"success\":true,\"message\":\"Server will restart in " + std::to_string(delay) + " seconds\"}";
        } else {
            response.body = R"({"success":true,"message":"Server is restarting"})";
        }
        
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during server restart: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AdminController::getLogs(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    try {
        // Parse query parameters
        std::string logLevel = "INFO"; // Default log level
        int lines = 100; // Default number of lines
        
        // Extract query parameters
        auto queryPos = request.uri.find('?');
        if (queryPos != std::string::npos) {
            std::string queryString = request.uri.substr(queryPos + 1);
            std::vector<std::string> pairs = utils::StringUtils::split(queryString, '&');
            
            for (const auto& pair : pairs) {
                std::vector<std::string> keyValue = utils::StringUtils::split(pair, '=');
                if (keyValue.size() == 2) {
                    if (keyValue[0] == "level") {
                        logLevel = keyValue[1];
                    } else if (keyValue[0] == "lines") {
                        try {
                            lines = std::stoi(keyValue[1]);
                            if (lines <= 0) lines = 100;
                            if (lines > 1000) lines = 1000; // Limit to 1000 lines
                        } catch (...) {
                            lines = 100;
                        }
                    }
                }
            }
        }
        
        // Get log file path
        std::string logPath = m_config->getServerConfig().logPath;
        
        // Check if file exists
        if (!utils::FileUtils::fileExists(logPath)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Log file not found");
        }
        
        // Read log file
        std::vector<std::string> allLines;
        bool success = utils::FileUtils::readFileLines(logPath, allLines);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to read log file");
        }
        
        // Filter by log level
        std::vector<std::string> filteredLines;
        for (const auto& line : allLines) {
            // Simple log level filtering (assumes log format like "[INFO]")
            if (logLevel == "ALL" || line.find("[" + logLevel + "]") != std::string::npos) {
                filteredLines.push_back(line);
            }
        }
        
        // Get the last 'lines' lines
        size_t startLine = 0;
        if (filteredLines.size() > static_cast<size_t>(lines)) {
            startLine = filteredLines.size() - static_cast<size_t>(lines);
        }
        
        std::vector<std::string> resultLines(filteredLines.begin() + startLine, filteredLines.end());
        
        // Create JSON response
        json response = {
            {"log_level", logLevel},
            {"total_lines", filteredLines.size()},
            {"returned_lines", resultLines.size()},
            {"logs", resultLines}
        };
        
        // Return response
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = response.dump(4); // Pretty print with 4 spaces indentation
        
        DEBUG_FUNCTION_EXIT();
        return httpResponse;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception getting logs: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

std::string AdminController::parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse) {
    DEBUG_FUNCTION_ENTER();
    
    // Check Content-Type header
    auto contentTypeIt = request.headers.find("Content-Type");
    if (contentTypeIt == request.headers.end() || contentTypeIt->second != "application/json") {
        errorResponse = createErrorResponse(400, "Content-Type must be application/json");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    // Check body
    if (request.body.empty()) {
        errorResponse = createErrorResponse(400, "Request body is empty");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    // Validate JSON syntax
    try {
        json::parse(request.body);
    }
    catch (const json::exception& e) {
        errorResponse = createErrorResponse(400, "Invalid JSON: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    
    DEBUG_FUNCTION_EXIT();
    return request.body;
}

HttpResponse AdminController::createErrorResponse(int statusCode, const std::string& message) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(statusCode);
    DEBUG_VARIABLE(message);
    
    HttpResponse response;
    response.statusCode = statusCode;
    
    switch (statusCode) {
        case 400: response.statusMessage = "Bad Request"; break;
        case 401: response.statusMessage = "Unauthorized"; break;
        case 403: response.statusMessage = "Forbidden"; break;
        case 404: response.statusMessage = "Not Found"; break;
        case 409: response.statusMessage = "Conflict"; break;
        case 500: response.statusMessage = "Internal Server Error"; break;
        default: response.statusMessage = "Error"; break;
    }
    
    response.headers["Content-Type"] = "application/json";
    
    // Create error JSON
    json error = {
        {"error", true},
        {"message", message}
    };
    
    response.body = error.dump();
    
    DEBUG_FUNCTION_EXIT();
    return response;
}

} // namespace rest_api
} // namespace d3server 