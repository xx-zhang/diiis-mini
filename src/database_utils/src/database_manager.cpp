#include <database_utils/database_manager.h>
#include <core/config.h>
#include <database_utils/crypto_utils.h>
#include <sqlite3.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>

namespace d3server {
namespace database_utils {

DatabaseManager::DatabaseManager(std::shared_ptr<core::Config> config)
    : m_config(config), m_db(nullptr) {
}

DatabaseManager::~DatabaseManager() {
    if (m_db) {
        sqlite3_close(static_cast<sqlite3*>(m_db));
        m_db = nullptr;
    }
}

bool DatabaseManager::init() {
    // Get database path from config
    std::string dbPath = m_config->getString("database.path", "d3server.db");
    
    // Open database connection
    int rc = sqlite3_open(dbPath.c_str(), reinterpret_cast<sqlite3**>(&m_db));
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(static_cast<sqlite3*>(m_db)) << std::endl;
        sqlite3_close(static_cast<sqlite3*>(m_db));
        m_db = nullptr;
        return false;
    }
    
    // Create tables if they don't exist
    return createTables();
}

bool DatabaseManager::createTables() {
    // Create accounts table
    std::string createAccountsTable = 
        "CREATE TABLE IF NOT EXISTS accounts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "login TEXT UNIQUE NOT NULL,"
        "email TEXT,"
        "password_hash TEXT NOT NULL,"
        "salt TEXT NOT NULL,"
        "is_banned INTEGER DEFAULT 0,"
        "last_login TEXT,"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "account_level INTEGER DEFAULT 0,"
        "battle_tag TEXT NOT NULL"
        ");";
    
    // Create characters table
    std::string createCharactersTable = 
        "CREATE TABLE IF NOT EXISTS characters ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "account_id INTEGER NOT NULL,"
        "name TEXT NOT NULL,"
        "class_id INTEGER NOT NULL,"
        "level INTEGER DEFAULT 1,"
        "experience INTEGER DEFAULT 0,"
        "is_hardcore INTEGER DEFAULT 0,"
        "is_deleted INTEGER DEFAULT 0,"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "gender INTEGER DEFAULT 0,"
        "flags INTEGER DEFAULT 0,"
        "last_played TEXT,"
        "FOREIGN KEY (account_id) REFERENCES accounts(id)"
        ");";
    
    // Create items table
    std::string createItemsTable = 
        "CREATE TABLE IF NOT EXISTS items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "owner_id INTEGER NOT NULL,"
        "item_id INTEGER NOT NULL,"
        "quality INTEGER DEFAULT 0,"
        "equipped_slot INTEGER DEFAULT -1,"
        "attributes TEXT,"
        "created_at TEXT NOT NULL,"
        "is_identified INTEGER DEFAULT 1,"
        "stack_count INTEGER DEFAULT 1,"
        "enchant_level INTEGER DEFAULT 0,"
        "FOREIGN KEY (owner_id) REFERENCES characters(id)"
        ");";
    
    // Create game_sessions table
    std::string createGameSessionsTable = 
        "CREATE TABLE IF NOT EXISTS game_sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "session_name TEXT NOT NULL,"
        "difficulty INTEGER DEFAULT 0,"
        "max_players INTEGER DEFAULT 4,"
        "is_public INTEGER DEFAULT 0,"
        "created_at TEXT NOT NULL,"
        "created_by_account_id INTEGER NOT NULL,"
        "game_mode TEXT DEFAULT 'Story',"
        "act_id INTEGER DEFAULT 0,"
        "FOREIGN KEY (created_by_account_id) REFERENCES accounts(id)"
        ");";
    
    // Create friends table
    std::string createFriendsTable = 
        "CREATE TABLE IF NOT EXISTS friends ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "account_id INTEGER NOT NULL,"
        "friend_account_id INTEGER NOT NULL,"
        "created_at TEXT NOT NULL,"
        "is_favorite INTEGER DEFAULT 0,"
        "FOREIGN KEY (account_id) REFERENCES accounts(id),"
        "FOREIGN KEY (friend_account_id) REFERENCES accounts(id)"
        ");";
    
    // Create clans table
    std::string createClansTable = 
        "CREATE TABLE IF NOT EXISTS clans ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "description TEXT,"
        "leader_account_id INTEGER NOT NULL,"
        "created_at TEXT NOT NULL,"
        "member_count INTEGER DEFAULT 0,"
        "tag TEXT NOT NULL,"
        "level INTEGER DEFAULT 1,"
        "FOREIGN KEY (leader_account_id) REFERENCES accounts(id)"
        ");";
    
    // Create clan_members table
    std::string createClanMembersTable = 
        "CREATE TABLE IF NOT EXISTS clan_members ("
        "clan_id INTEGER NOT NULL,"
        "account_id INTEGER NOT NULL,"
        "joined_at TEXT NOT NULL,"
        "rank INTEGER DEFAULT 0,"
        "PRIMARY KEY (clan_id, account_id),"
        "FOREIGN KEY (clan_id) REFERENCES clans(id),"
        "FOREIGN KEY (account_id) REFERENCES accounts(id)"
        ");";
    
    // Execute all create table queries
    bool success = true;
    success &= executeQuery(createAccountsTable);
    success &= executeQuery(createCharactersTable);
    success &= executeQuery(createItemsTable);
    success &= executeQuery(createGameSessionsTable);
    success &= executeQuery(createFriendsTable);
    success &= executeQuery(createClansTable);
    success &= executeQuery(createClanMembersTable);
    
    return success;
}

bool DatabaseManager::executeQuery(const std::string& query, const std::vector<std::pair<std::string, std::string>>& params) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(m_db), query.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(m_db)) << std::endl;
        return false;
    }
    
    // Bind parameters if any
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i].first == "int") {
            sqlite3_bind_int(stmt, i + 1, std::stoi(params[i].second));
        } else if (params[i].first == "int64") {
            sqlite3_bind_int64(stmt, i + 1, std::stoll(params[i].second));
        } else if (params[i].first == "double") {
            sqlite3_bind_double(stmt, i + 1, std::stod(params[i].second));
        } else if (params[i].first == "null") {
            sqlite3_bind_null(stmt, i + 1);
        } else {
            sqlite3_bind_text(stmt, i + 1, params[i].second.c_str(), -1, SQLITE_STATIC);
        }
    }
    
    // Execute statement
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE || rc == SQLITE_ROW);
}

std::string DatabaseManager::generateSalt() {
    // Use CryptoUtils to generate salt
    return CryptoUtils::generateSalt();
}

std::string DatabaseManager::hashPassword(const std::string& password, const std::string& salt) {
    // Use CryptoUtils to hash password
    return CryptoUtils::hashPassword(password, salt);
}

std::string DatabaseManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool DatabaseManager::accountExists(const std::string& login) {
    std::string query = "SELECT COUNT(*) FROM accounts WHERE login = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(m_db), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count > 0;
}

bool DatabaseManager::verifyAccountPassword(const std::string& login, const std::string& password) {
    std::string query = "SELECT password_hash, salt FROM accounts WHERE login = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(m_db), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    bool result = false;
    
    if (rc == SQLITE_ROW) {
        std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        
        std::string computedHash = hashPassword(password, salt);
        result = (computedHash == storedHash);
    }
    
    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseManager::createAccount(const std::string& login, const std::string& email, const std::string& password, const std::string& battleTag) {
    if (accountExists(login)) {
        return false;
    }
    
    std::string salt = generateSalt();
    std::string passwordHash = hashPassword(password, salt);
    std::string timestamp = getCurrentTimestamp();
    
    std::string query = 
        "INSERT INTO accounts (login, email, password_hash, salt, created_at, updated_at, battle_tag) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    std::vector<std::pair<std::string, std::string>> params = {
        {"text", login},
        {"text", email},
        {"text", passwordHash},
        {"text", salt},
        {"text", timestamp},
        {"text", timestamp},
        {"text", battleTag}
    };
    
    return executeQuery(query, params);
}

// Implement other methods as needed...

} // namespace database_utils
} // namespace d3server 
