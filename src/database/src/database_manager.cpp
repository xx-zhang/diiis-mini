#include "database/database_manager.h"
#include "core/logger.h"
#include "utils/debug.h"
#include <sqlite3.h>
#include <stdexcept>
#include <vector>
#include "utils/crypto_utils.h"

namespace d3server {
namespace database {

DatabaseManager::DatabaseManager(std::shared_ptr<core::Config> config)
    : m_config(config), m_db(nullptr) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_FUNCTION_EXIT();
}

DatabaseManager::~DatabaseManager() {
    DEBUG_FUNCTION_ENTER();
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
        LOG_INFO("Database connection closed.");
    }
    DEBUG_FUNCTION_EXIT();
}

bool DatabaseManager::init() {
    DEBUG_FUNCTION_ENTER();
    core::DatabaseConfig dbConfig = m_config->getDatabaseConfig();
    std::string dbPath = dbConfig.filePath; // Changed dbName to filePath

    LOG_INFO("Initializing database: " + dbPath);

    if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK) {
        LOG_ERROR("Failed to open database: " + std::string(sqlite3_errmsg(m_db)));
        DEBUG_FUNCTION_EXIT();
        return false;
    }

    LOG_INFO("Database initialized successfully: " + dbPath);
    // TODO: Create tables if they don't exist
    createTables(); 
    DEBUG_FUNCTION_EXIT();
    return true;
}

// Account Management
bool DatabaseManager::accountExists(const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    sqlite3_stmt* stmt;
    std::string sql = "SELECT id FROM accounts WHERE login = ?;";
    
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement for accountExists: " + std::string(sqlite3_errmsg(m_db)));
        DEBUG_FUNCTION_EXIT();
        return false; // Or throw exception
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = true;
    }

    sqlite3_finalize(stmt);
    DEBUG_LOG(login + (exists ? " exists." : " does not exist."));
    DEBUG_FUNCTION_EXIT();
    return exists;
}

bool DatabaseManager::verifyAccountPassword(const std::string& login, const std::string& password) {
    DEBUG_FUNCTION_ENTER();
    sqlite3_stmt* stmt;
    std::string sql = "SELECT password_hash, salt FROM accounts WHERE login = ?;";

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement for verifyAccountPassword: " + std::string(sqlite3_errmsg(m_db)));
        DEBUG_FUNCTION_EXIT();
        return false;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

    bool verified = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* db_password_hash_unsigned = sqlite3_column_text(stmt, 0);
        const unsigned char* db_salt_unsigned = sqlite3_column_text(stmt, 1);

        if (db_password_hash_unsigned && db_salt_unsigned) {
            std::string db_password_hash = reinterpret_cast<const char*>(db_password_hash_unsigned);
            std::string db_salt = reinterpret_cast<const char*>(db_salt_unsigned);
            
            // For now, direct comparison. Later, use CryptoUtils::verifyPassword
            // verified = (db_password_hash == password); 
            // The 'password' parameter is plain text, it needs to be hashed with db_salt first.
            // This will be correctly implemented once CryptoUtils is fully integrated for password hashing.
            // Placeholder for actual verification logic using CryptoUtils::verifyPassword
            // For now, if we find the user, we assume password is correct for initial testing.
            // This is a **SECURITY RISK** and needs to be replaced.
            // verified = true; // TEMPORARY
            
            // Corrected (but still needs CryptoUtils to be fully working for hashing new passwords)
            verified = utils::CryptoUtils::verifyPassword(password, db_salt, db_password_hash);
            if (verified) {
                LOG_INFO("Password verified for user: " + login);
            } else {
                LOG_WARNING("Password verification failed for user: " + login);
            }
        } else {
            LOG_ERROR("Password hash or salt is null for user: " + login);
        }
    } else {
        LOG_WARNING("User not found during password verification: " + login);
    }

    sqlite3_finalize(stmt);
    DEBUG_FUNCTION_EXIT();
    return verified;
}

bool DatabaseManager::createAccount(const std::string& login, const std::string& email, const std::string& plainPassword) {
    DEBUG_FUNCTION_ENTER();

    if (accountExists(login)) {
        LOG_WARNING("Attempt to create account that already exists: " + login);
        DEBUG_FUNCTION_EXIT();
        return false;
    }

    // Generate salt and hash password
    std::string salt = utils::CryptoUtils::generateRandomString(16); // Generate a 16-byte salt
    std::string hashedPassword = utils::CryptoUtils::hashPassword(plainPassword, salt);

    sqlite3_stmt* stmt;
    std::string sql = "INSERT INTO accounts (login, email, password_hash, salt, last_login) VALUES (?, ?, ?, ?, datetime('now'));";
    
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement for createAccount: " + std::string(sqlite3_errmsg(m_db)));
        DEBUG_FUNCTION_EXIT();
        return false;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, salt.c_str(), -1, SQLITE_STATIC);

    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        LOG_INFO("Account created successfully: " + login);
        success = true;
    } else {
        LOG_ERROR("Failed to create account " + login + ": " + std::string(sqlite3_errmsg(m_db)));
    }

    sqlite3_finalize(stmt);
    DEBUG_FUNCTION_EXIT();
    return success;
}

bool DatabaseManager::isAccountBanned(const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    sqlite3_stmt* stmt;
    std::string sql = "SELECT banned FROM accounts WHERE login = ?;";

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement for isAccountBanned: " + std::string(sqlite3_errmsg(m_db)));
        DEBUG_FUNCTION_EXIT();
        return false; // Consider true by default or throw an error
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

    bool banned = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        banned = (sqlite3_column_int(stmt, 0) == 1);
    }

    sqlite3_finalize(stmt);
    DEBUG_LOG(login + (banned ? " is banned." : " is not banned."));
    DEBUG_FUNCTION_EXIT();
    return banned;
}

bool DatabaseManager::updateAccountLastLogin(const std::string& login) {
    DEBUG_FUNCTION_ENTER();
    sqlite3_stmt* stmt;
    std::string sql = "UPDATE accounts SET last_login = datetime('now') WHERE login = ?;";

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement for updateAccountLastLogin: " + std::string(sqlite3_errmsg(m_db)));
        DEBUG_FUNCTION_EXIT();
        return false;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        LOG_INFO("Updated last_login for account: " + login);
        success = true;
    } else {
        LOG_ERROR("Failed to update last_login for account " + login + ": " + std::string(sqlite3_errmsg(m_db)));
    }

    sqlite3_finalize(stmt);
    DEBUG_FUNCTION_EXIT();
    return success;
}

// Placeholder for other methods that were in the original description
// but are not directly called by BattleNetClient.cpp
// These would need to be implemented based on original C# logic.

// bool DatabaseManager::deleteAccount(const std::string& login);
// bool DatabaseManager::updateAccountEmail(const std::string& login, const std::string& newEmail);
// bool DatabaseManager::updateAccountPassword(const std::string& login, const std::string& newPassword);
// bool DatabaseManager::banAccount(const std::string& login, const std::string& reason, bool ban);
// std::vector<Account> DatabaseManager::getAllAccounts();
// bool DatabaseManager::getAccount(const std::string& login);

// Character Management
// std::vector<Character> DatabaseManager::getCharactersForAccount(const std::string& login);
// bool DatabaseManager::createCharacter(const std::string& accountLogin, const CharacterData& data);
// bool DatabaseManager::deleteCharacter(const std::string& accountLogin, const std::string& characterName);

// Private helper for table creation
bool DatabaseManager::createTables() {
    DEBUG_FUNCTION_ENTER();
    
    const char* createAccountsTableSQL = R"SQL(
    CREATE TABLE IF NOT EXISTS accounts (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        login TEXT UNIQUE NOT NULL,
        email TEXT UNIQUE NOT NULL,
        password_hash TEXT NOT NULL,
        salt TEXT NOT NULL, -- Store salt for password hashing
        banned INTEGER DEFAULT 0,
        last_login TIMESTAMP,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );)SQL";

    if (!executeQuery(createAccountsTableSQL, {}, false)) {
        LOG_ERROR("Failed to create accounts table.");
        DEBUG_FUNCTION_EXIT();
        return false;
    }

    const char* createCharactersTableSQL = R"SQL(
    CREATE TABLE IF NOT EXISTS characters (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        account_id INTEGER NOT NULL,
        name TEXT NOT NULL,
        class TEXT NOT NULL,
        level INTEGER DEFAULT 1,
        experience INTEGER DEFAULT 0,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE
    );
    CREATE INDEX IF NOT EXISTS idx_character_account_id ON characters(account_id);
    CREATE UNIQUE INDEX IF NOT EXISTS idx_character_account_name ON characters(account_id, name);
    )SQL";

    if (!executeQuery(createCharactersTableSQL, {}, false)) {
        LOG_ERROR("Failed to create characters table.");
        DEBUG_FUNCTION_EXIT();
        return false;
    }

    // Add more table creation statements here...
    // e.g., items, game_instances, etc.

    LOG_INFO("Database tables created/verified successfully.");
    DEBUG_FUNCTION_EXIT();
    return true;
}

bool DatabaseManager::executeQuery(const std::string& query_string, 
                                 const std::vector<std::pair<std::string, std::string>>& params,
                                 bool expect_results) {
    DEBUG_FUNCTION_ENTER();
    
    if (!params.empty()) {
        LOG_ERROR("executeQuery with parameters is not fully implemented yet for non-prepared statements.");
        // For now, we'll proceed without binding, which is incorrect if params are meant for this query_string.
        // This part needs proper prepared statement logic if params are to be used.
    }

    if (expect_results) {
        LOG_WARNING("executeQuery is called with expect_results=true, but this basic version doesn't return results, only success/failure.");
        // This part needs logic to step through results if expect_results is true.
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, query_string.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        LOG_ERROR("SQL error: " + query_string + " - " + std::string(errMsg));
        sqlite3_free(errMsg);
        DEBUG_FUNCTION_EXIT();
        return false;
    }
    
    DEBUG_FUNCTION_EXIT();
    return true;
}

} // namespace database
} // namespace d3server 