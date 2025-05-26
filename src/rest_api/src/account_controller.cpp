#include "rest_api/account_controller.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "database/database_manager.h"

#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

namespace d3server {
namespace rest_api {

AccountController::AccountController(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AccountController created");
    DEBUG_FUNCTION_EXIT();
}

AccountController::~AccountController() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("AccountController destroyed");
    DEBUG_FUNCTION_EXIT();
}

HttpResponse AccountController::createAccount(const HttpRequest& request) {
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
    
    // Validate account data
    std::string errorMessage;
    if (!validateAccountData(jsonStr, errorMessage)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, errorMessage);
    }
    
    try {
        // Parse account data
        json accountData = json::parse(jsonStr);
        
        // Extract required fields
        std::string login = accountData["login"];
        std::string email = accountData["email"];
        std::string password = accountData["password"];
        
        // Check if account already exists
        if (m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(409, "Account with this login already exists");
        }
        
        // Create account
        bool success = m_dbManager->createAccount(login, email, password);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to create account");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 201; // Created
        response.statusMessage = "Created";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Account created successfully"})";
        
        LOG_INFO("Account created: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception creating account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::getAllAccounts(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    try {
        // Get accounts from database
        std::vector<database::Account> accounts = m_dbManager->getAllAccounts();
        
        // Convert to JSON
        json response = json::array();
        for (const auto& account : accounts) {
            json accountJson = {
                {"login", account.login},
                {"email", account.email},
                {"created", account.created},
                {"last_login", account.lastLogin},
                {"banned", account.banned},
                {"character_count", account.characterCount}
            };
            response.push_back(accountJson);
        }
        
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
        LOG_ERROR("Exception getting all accounts: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::getAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Get account from database
        database::Account account = m_dbManager->getAccount(login);
        
        // Convert to JSON
        json response = {
            {"login", account.login},
            {"email", account.email},
            {"created", account.created},
            {"last_login", account.lastLogin},
            {"banned", account.banned},
            {"character_count", account.characterCount}
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
        LOG_ERROR("Exception getting account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::updateAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    // Check if account exists
    if (!m_dbManager->accountExists(login)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(404, "Account not found");
    }
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        // Parse account data
        json accountData = json::parse(jsonStr);
        
        // Prepare update data - only email and password can be updated
        std::string newEmail = accountData.contains("email") ? accountData["email"] : "";
        std::string newPassword = accountData.contains("password") ? accountData["password"] : "";
        
        // Validate email if provided
        if (!newEmail.empty()) {
            // Simple email validation with regex
            std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
            if (!std::regex_match(newEmail, emailRegex)) {
                DEBUG_FUNCTION_EXIT();
                return createErrorResponse(400, "Invalid email format");
            }
        }
        
        // Validate password if provided
        if (!newPassword.empty() && newPassword.length() < 8) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(400, "Password must be at least 8 characters long");
        }
        
        // Update account
        bool success = m_dbManager->updateAccount(login, newEmail, newPassword);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to update account");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Account updated successfully"})";
        
        LOG_INFO("Account updated: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception updating account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::deleteAccount(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Delete account
        bool success = m_dbManager->deleteAccount(login);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to delete account");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Account deleted successfully"})";
        
        LOG_INFO("Account deleted: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception deleting account: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::getCharacters(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        // Get characters from database
        std::vector<database::Character> characters = m_dbManager->getCharacters(login);
        
        // Convert to JSON
        json response = json::array();
        for (const auto& character : characters) {
            json characterJson = {
                {"id", character.id},
                {"name", character.name},
                {"class_id", character.classId},
                {"level", character.level},
                {"created", character.created},
                {"last_played", character.lastPlayed}
            };
            response.push_back(characterJson);
        }
        
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
        LOG_ERROR("Exception getting characters: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AccountController::createCharacter(const HttpRequest& request, const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    DEBUG_VARIABLE(login);
    
    // Check if account exists
    if (!m_dbManager->accountExists(login)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(404, "Account not found");
    }
    
    // Check character limit
    if (m_dbManager->getCharacterCount(login) >= m_config->getServerConfig().maxCharactersPerAccount) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, "Maximum number of characters reached for this account");
    }
    
    // Parse JSON from request body
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    // Validate character data
    std::string errorMessage;
    if (!validateCharacterData(jsonStr, errorMessage)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, errorMessage);
    }
    
    try {
        // Parse character data
        json characterData = json::parse(jsonStr);
        
        // Extract required fields
        std::string name = characterData["name"];
        int classId = characterData["class_id"];
        
        // Check if character name already exists
        if (m_dbManager->characterExists(name)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(409, "Character with this name already exists");
        }
        
        // Create character
        database::Character character;
        character.name = name;
        character.classId = classId;
        character.level = 1; // Start at level 1
        
        bool success = m_dbManager->createCharacter(login, character);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to create character");
        }
        
        // Return success response
        HttpResponse response;
        response.statusCode = 201; // Created
        response.statusMessage = "Created";
        response.headers["Content-Type"] = "application/json";
        response.body = R"({"success":true,"message":"Character created successfully"})";
        
        LOG_INFO("Character created: " + name + " for account: " + login);
        DEBUG_FUNCTION_EXIT();
        return response;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception creating character: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(500, "Internal server error");
    }
}

bool AccountController::validateAccountData(const std::string& jsonStr, std::string& errorMessage) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        json accountData = json::parse(jsonStr);
        
        // Check required fields
        if (!accountData.contains("login")) {
            errorMessage = "Missing required field: login";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        if (!accountData.contains("email")) {
            errorMessage = "Missing required field: email";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        if (!accountData.contains("password")) {
            errorMessage = "Missing required field: password";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate login
        std::string login = accountData["login"];
        if (login.empty() || login.length() < 3 || login.length() > 16) {
            errorMessage = "Login must be between 3 and 16 characters";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate login characters (alphanumeric)
        std::regex loginRegex("^[a-zA-Z0-9]+$");
        if (!std::regex_match(login, loginRegex)) {
            errorMessage = "Login can only contain alphanumeric characters";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate email
        std::string email = accountData["email"];
        std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        if (!std::regex_match(email, emailRegex)) {
            errorMessage = "Invalid email format";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate password
        std::string password = accountData["password"];
        if (password.length() < 8) {
            errorMessage = "Password must be at least 8 characters long";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const json::exception& e) {
        errorMessage = "Invalid JSON: " + std::string(e.what());
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

bool AccountController::validateCharacterData(const std::string& jsonStr, std::string& errorMessage) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        json characterData = json::parse(jsonStr);
        
        // Check required fields
        if (!characterData.contains("name")) {
            errorMessage = "Missing required field: name";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        if (!characterData.contains("class_id")) {
            errorMessage = "Missing required field: class_id";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate name
        std::string name = characterData["name"];
        if (name.empty() || name.length() < 2 || name.length() > 12) {
            errorMessage = "Name must be between 2 and 12 characters";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate name characters (alphanumeric and some special chars)
        std::regex nameRegex("^[a-zA-Z0-9_-]+$");
        if (!std::regex_match(name, nameRegex)) {
            errorMessage = "Name can only contain alphanumeric characters, underscores, and hyphens";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Validate class_id
        int classId = characterData["class_id"];
        if (classId < 1 || classId > 7) { // Assuming 7 classes in Diablo 3
            errorMessage = "Invalid class_id. Must be between 1 and 7";
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const json::exception& e) {
        errorMessage = "Invalid JSON: " + std::string(e.what());
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

std::string AccountController::parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse) {
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

HttpResponse AccountController::createErrorResponse(int statusCode, const std::string& message) {
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