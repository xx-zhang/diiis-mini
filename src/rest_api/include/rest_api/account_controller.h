#pragma once

#include "rest_api/rest_server.h"
#include "database_utils/database_manager.h"

#include <memory>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database_utils {
class DatabaseManager;
}

namespace rest_api {

/**
 * @brief Controller for account-related API endpoints
 */
class AccountController {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    AccountController(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database_utils::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~AccountController();
    
    /**
     * @brief Create a new account
     * @param request HTTP request containing account data
     * @return HTTP response
     */
    HttpResponse createAccount(const HttpRequest& request);
    
    /**
     * @brief Get all accounts
     * @param request HTTP request
     * @return HTTP response containing a list of accounts
     */
    HttpResponse getAllAccounts(const HttpRequest& request);
    
    /**
     * @brief Get account by login
     * @param request HTTP request
     * @param login Account login
     * @return HTTP response containing account data
     */
    HttpResponse getAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Update an account
     * @param request HTTP request containing updated account data
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse updateAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Delete an account
     * @param request HTTP request
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse deleteAccount(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Get characters for an account
     * @param request HTTP request
     * @param login Account login
     * @return HTTP response containing a list of characters
     */
    HttpResponse getCharacters(const HttpRequest& request, const std::string& login);
    
    /**
     * @brief Create a new character for an account
     * @param request HTTP request containing character data
     * @param login Account login
     * @return HTTP response
     */
    HttpResponse createCharacter(const HttpRequest& request, const std::string& login);
    
private:
    /**
     * @brief Helper to validate account data
     * @param json Account data as JSON
     * @param errorMessage Output variable for validation error message
     * @return True if the account data is valid, false otherwise
     */
    bool validateAccountData(const std::string& json, std::string& errorMessage);
    
    /**
     * @brief Helper to validate character data
     * @param json Character data as JSON
     * @param errorMessage Output variable for validation error message
     * @return True if the character data is valid, false otherwise
     */
    bool validateCharacterData(const std::string& json, std::string& errorMessage);
    
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
    std::shared_ptr<database_utils::DatabaseManager> m_dbManager;
};

} // namespace rest_api
} // namespace d3server 
