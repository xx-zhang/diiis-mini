#include "rest_api/account_controller.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "database_utils/database_manager.h"

#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

namespace d3server {
namespace rest_api {

AccountController::AccountController(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database_utils::DatabaseManager> dbManager
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
    
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    std::string errorMessage;
    if (!validateAccountData(jsonStr, errorMessage)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, errorMessage);
    }
    
    try {
        json accountData = json::parse(jsonStr);
        std::string login = accountData["login"];
        std::string email = accountData["email"];
        std::string password = accountData["password"];
        std::string battleTag = accountData.contains("battle_tag") ? accountData["battle_tag"].get<std::string>() : login;

        if (m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(409, "Account with this login already exists");
        }
        
        bool success = m_dbManager->createAccount(login, email, password, battleTag);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to create account");
        }
        
        HttpResponse response;
        response.statusCode = 201;
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
        std::vector<database_utils::AccountData> accounts;
        
        json responseJson = json::array();
        for (const auto& account : accounts) {
            json accountJson = {
                {"login", account.login},
                {"email", account.email},
                {"created_at", account.created_at},
                {"last_login", account.last_login},
                {"is_banned", account.is_banned},
                {"battle_tag", account.battle_tag}
            };
            responseJson.push_back(accountJson);
        }
        
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = responseJson.dump(4);
        
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
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        std::optional<database_utils::AccountData> accountOpt = m_dbManager->getAccountDetails(login);
        if (!accountOpt) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found after existence check");
        }
        database_utils::AccountData account = accountOpt.value();
        
        json responseJson = {
            {"login", account.login},
            {"email", account.email},
            {"created_at", account.created_at},
            {"last_login", account.last_login},
            {"is_banned", account.is_banned},
            {"battle_tag", account.battle_tag}
        };
        
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = responseJson.dump(4);
        
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
    
    if (!m_dbManager->accountExists(login)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(404, "Account not found");
    }
    
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    try {
        json accountDataJson = json::parse(jsonStr);
        if (accountDataJson.contains("password")) {
            std::string newPassword = accountDataJson["password"];
            if (newPassword.length() < 8) {
                DEBUG_FUNCTION_EXIT();
                return createErrorResponse(400, "Password must be at least 8 characters long");
            }
            
            bool success = m_dbManager->changeAccountPassword(login, newPassword);
            if (!success) {
                DEBUG_FUNCTION_EXIT();
                return createErrorResponse(500, "Failed to update account password");
            }
        }
        
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
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        bool success = m_dbManager->deleteAccount(login);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to delete account");
        }
        
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
        if (!m_dbManager->accountExists(login)) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account not found");
        }
        
        std::optional<database_utils::AccountData> accountOpt = m_dbManager->getAccountDetails(login);
        if (!accountOpt) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(404, "Account details not found for character list retrieval");
        }
        int accountId = accountOpt.value().id;
        
        std::vector<database_utils::CharacterData> characters = m_dbManager->getCharactersForAccount(accountId);
        
        json responseJson = json::array();
        for (const auto& character : characters) {
            json characterJson = {
                {"id", character.id},
                {"name", character.name},
                {"class_id", character.class_id},
                {"level", character.level},
                {"created_at", character.created_at},
                {"last_played", character.last_played}
            };
            responseJson.push_back(characterJson);
        }
        
        HttpResponse httpResponse;
        httpResponse.statusCode = 200;
        httpResponse.statusMessage = "OK";
        httpResponse.headers["Content-Type"] = "application/json";
        httpResponse.body = responseJson.dump(4);
        
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
    
    std::optional<database_utils::AccountData> accountOpt = m_dbManager->getAccountDetails(login);
    if (!accountOpt) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(404, "Account not found for character creation");
    }
    int accountId = accountOpt.value().id;

    std::vector<database_utils::CharacterData> existing_characters = m_dbManager->getCharactersForAccount(accountId);
    if (existing_characters.size() >= m_config->getServerConfig().maxCharactersPerAccount) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, "Maximum number of characters reached for this account");
    }
    
    HttpResponse errorResponse;
    std::string jsonStr = parseRequestJson(request, errorResponse);
    if (jsonStr.empty()) {
        DEBUG_FUNCTION_EXIT();
        return errorResponse;
    }
    
    std::string errorMessage;
    if (!validateCharacterData(jsonStr, errorMessage)) {
        DEBUG_FUNCTION_EXIT();
        return createErrorResponse(400, errorMessage);
    }
    
    try {
        json characterDataJson = json::parse(jsonStr);
        std::string name = characterDataJson["name"];
        int classId = characterDataJson["class_id"];
        int gender = characterDataJson.contains("gender") ? characterDataJson["gender"].get<int>() : 0;
        bool hardcore = characterDataJson.contains("is_hardcore") ? characterDataJson["is_hardcore"].get<bool>() : false;

        bool name_exists = false;
        std::vector<database_utils::CharacterData> all_chars_for_account = m_dbManager->getCharactersForAccount(accountId);
        for(const auto& c : all_chars_for_account) {
            if (c.name == name) {
                name_exists = true;
                break;
            }
        }
        if (name_exists) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(409, "Character with this name already exists for this account");
        }
        
        bool success = m_dbManager->createCharacter(accountId, name, classId, gender, hardcore);
        if (!success) {
            DEBUG_FUNCTION_EXIT();
            return createErrorResponse(500, "Failed to create character");
        }
        
        HttpResponse response;
        response.statusCode = 201;
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
        if (!accountData.contains("login")) { errorMessage = "Missing required field: login"; DEBUG_FUNCTION_EXIT(); return false; }
        if (!accountData.contains("email")) { errorMessage = "Missing required field: email"; DEBUG_FUNCTION_EXIT(); return false; }
        if (!accountData.contains("password")) { errorMessage = "Missing required field: password"; DEBUG_FUNCTION_EXIT(); return false; }

        std::string login = accountData["login"];
        if (login.empty() || login.length() < 3 || login.length() > 16) { errorMessage = "Login must be between 3 and 16 characters"; DEBUG_FUNCTION_EXIT(); return false; }
        std::regex loginRegex("^[a-zA-Z0-9]+$");
        if (!std::regex_match(login, loginRegex)) { errorMessage = "Login can only contain alphanumeric characters"; DEBUG_FUNCTION_EXIT(); return false; }
        
        std::string email = accountData["email"];
        std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        if (!std::regex_match(email, emailRegex)) { errorMessage = "Invalid email format"; DEBUG_FUNCTION_EXIT(); return false; }
        
        std::string password = accountData["password"];
        if (password.length() < 8) { errorMessage = "Password must be at least 8 characters long"; DEBUG_FUNCTION_EXIT(); return false; }
        
        DEBUG_FUNCTION_EXIT();
        return true;
    } catch (const json::exception& e) {
        errorMessage = "Invalid JSON: " + std::string(e.what());
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

bool AccountController::validateCharacterData(const std::string& jsonStr, std::string& errorMessage) {
    DEBUG_FUNCTION_ENTER();
    try {
        json characterData = json::parse(jsonStr);
        if (!characterData.contains("name")) { errorMessage = "Missing required field: name"; DEBUG_FUNCTION_EXIT(); return false; }
        if (!characterData.contains("class_id")) { errorMessage = "Missing required field: class_id"; DEBUG_FUNCTION_EXIT(); return false; }

        std::string name = characterData["name"];
        if (name.empty() || name.length() < 2 || name.length() > 12) { errorMessage = "Name must be between 2 and 12 characters"; DEBUG_FUNCTION_EXIT(); return false; }
        std::regex nameRegex("^[a-zA-Z0-9_-]+$");
        if (!std::regex_match(name, nameRegex)) { errorMessage = "Name can only contain alphanumeric characters, underscores, and hyphens"; DEBUG_FUNCTION_EXIT(); return false; }
        
        int classId = characterData["class_id"];
        if (classId < 1 || classId > 7) { errorMessage = "Invalid class_id. Must be between 1 and 7"; DEBUG_FUNCTION_EXIT(); return false; }
        
        if (characterData.contains("gender")) {
            if (!characterData["gender"].is_number_integer()) {errorMessage = "gender must be an integer"; DEBUG_FUNCTION_EXIT(); return false;}
        }
        if (characterData.contains("is_hardcore")) {
            if (!characterData["is_hardcore"].is_boolean()) {errorMessage = "is_hardcore must be a boolean"; DEBUG_FUNCTION_EXIT(); return false;}
        }

        DEBUG_FUNCTION_EXIT();
        return true;
    } catch (const json::exception& e) {
        errorMessage = "Invalid JSON: " + std::string(e.what());
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

std::string AccountController::parseRequestJson(const HttpRequest& request, HttpResponse& errorResponse) {
    DEBUG_FUNCTION_ENTER();
    auto contentTypeIt = request.headers.find("Content-Type");
    if (contentTypeIt == request.headers.end() || contentTypeIt->second.find("application/json") == std::string::npos) {
        errorResponse = createErrorResponse(400, "Content-Type must be application/json");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    if (request.body.empty()) {
        errorResponse = createErrorResponse(400, "Request body is empty");
        DEBUG_FUNCTION_EXIT();
        return "";
    }
    try {
        json::parse(request.body);
    } catch (const json::exception& e) {
        errorResponse = createErrorResponse(400, "Invalid JSON syntax: " + std::string(e.what()));
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
    json error = { {"error", true}, {"message", message} };
    response.body = error.dump();
    DEBUG_FUNCTION_EXIT();
    return response;
}

} // namespace rest_api
} // namespace d3server 
