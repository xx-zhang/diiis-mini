#pragma once

#include "rest_api/rest_server.h"

#include <memory>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace rest_api {

/**
 * @brief Controller for administrative API endpoints
 */
class AdminController {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    AdminController(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~AdminController();
    
    /**
     * @brief Ban/unban an account
     * @param request HTTP request containing ban data
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse banAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Get server status
     * @param request HTTP request
     * @return HTTP response containing server status
     */
    HttpResponse getServerStatus(const HttpRequest& request);
    
    /**
     * @brief Shutdown the server
     * @param request HTTP request
     * @return HTTP response
     */
    HttpResponse shutdownServer(const HttpRequest& request);
    
    /**
     * @brief Restart the server
     * @param request HTTP request
     * @return HTTP response
     */
    HttpResponse restartServer(const HttpRequest& request);
    
    /**
     * @brief Get server logs
     * @param request HTTP request
     * @return HTTP response containing logs
     */
    HttpResponse getLogs(const HttpRequest& request);
    
private:
    /**
     * @brief Parse JSON from request body
     * @param request HTTP request
     * @param errorResponse Output variable for error response
     * @return JSON string if valid, empty string if invalid
     */
    std::string parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse);
    
    /**
     * @brief Create a JSON error response
     * @param statusCode HTTP status code
     * @param message Error message
     * @return HTTP response
     */
    HttpResponse createErrorResponse(int statusCode, const std::string& message);
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
};

} // namespace rest_api
} // namespace d3server 