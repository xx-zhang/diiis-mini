#ifndef D3SERVER_DATABASE_MANAGER_H
#define D3SERVER_DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include "sqlite3.h" // Assuming direct SQLite C API usage
#include "core/logger.h" // For logging
#include "utils/debug.h"   // For DEBUG_ macros
#include "utils/crypto_utils.h" // For password hashing utilities
#include "core/config.h" // Added for core::Config

// Forward declaration if needed, e.g., for Config
// namespace d3server { namespace core { class Config; } }

namespace d3server {
namespace core { // Forward declaration is not enough if used in shared_ptr directly in header
    // class Config; // This would be enough if only pointers/references used in method signatures
}
namespace database {

// Optional: Struct to hold account data if returned by methods
struct AccountData {
    uint32_t id;
    std::string login;
    std::string email;
    std::string password_hash;
    std::string salt;
    bool is_banned;
    std::string last_login;
    std::string created_at;
    std::string updated_at;
};

// Optional: Struct to hold character data if returned by methods
struct CharacterData {
    uint64_t id;
    uint32_t account_id;
    std::string name;
    // Add other fields like class, level, experience, etc.
    std::string created_at;
    std::string updated_at;
};


class DatabaseManager {
public:
    DatabaseManager(std::shared_ptr<core::Config> config); // Changed signature
    ~DatabaseManager();

    bool init(); // Changed signature, gets path from config member
    bool createTables(); // Kept as bool, will ensure .cpp returns bool

    // Account Management
    bool accountExists(const std::string& login);
    bool verifyAccountPassword(const std::string& login, const std::string& password);
    bool createAccount(const std::string& login, const std::string& email, const std::string& password);
    bool isAccountBanned(const std::string& login);
    bool updateAccountLastLogin(const std::string& login);
    // Add other account methods as needed, e.g.:
    // std::optional<AccountData> getAccountByLogin(const std::string& login);
    // bool setAccountBanStatus(const std::string& login, bool banned);
    // bool changeAccountPassword(const std::string& login, const std::string& new_password);
    // bool deleteAccount(const std::string& login);


    // Character Management (assuming these will be added)
    // bool createCharacter(uint32_t account_id, const std::string& name, const std::string& char_class /*, other details*/);
    // std::vector<CharacterData> getCharactersForAccount(uint32_t account_id);
    // std::optional<CharacterData> getCharacterById(uint64_t character_id);
    // bool deleteCharacter(uint64_t character_id);
    // bool updateCharacterData(const CharacterData& data); // For saving progress

    // Generic query execution (if needed publicly, else keep private)
    // bool executeQuery(const std::string& sql); 

private:
    sqlite3* m_db;
    // std::string m_dbPath; // Path will come from m_config
    std::shared_ptr<core::Config> m_config; // Added config member

    // Private helper for SQL execution, matching the .cpp
    bool executeQuery(const std::string& query_string, const std::vector<std::pair<std::string, std::string>>& params = {}, bool expect_results = false);
    
    // Helper for preparing statements (example, can be more generic)
    sqlite3_stmt* prepareStatement(const std::string& query);
};

} // namespace database
} // namespace d3server

#endif // D3SERVER_DATABASE_MANAGER_H 