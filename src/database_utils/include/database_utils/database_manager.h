#ifndef D3SERVER_DATABASE_MANAGER_H
#define D3SERVER_DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>
#include "core/logger.h"
#include "core/config.h"
#include "database_utils/crypto_utils.h"

// Forward declarations for core components
namespace d3server {
namespace core {
    class Logger;
    class Config;
}
}

// Forward declare sqlite_orm namespace if using it
namespace sqlite_orm {
    template<typename... Args>
    struct storage;
}

namespace d3server {
namespace database_utils {

// Account data structure
struct AccountData {
    int id = 0;
    std::string login;
    std::string email;
    std::string password_hash;
    std::string salt;
    bool is_banned = false;
    std::string last_login;
    std::string created_at;
    std::string updated_at;
    int account_level = 0; // Normal, VIP, Admin, etc.
    std::string battle_tag; // BattleTag for the account
};

// Character data structure
struct CharacterData {
    int64_t id = 0;
    int account_id = 0;
    std::string name;
    int class_id = 0;    // Character class (Barbarian, Wizard, etc.)
    int level = 1;       // Character level
    int64_t experience = 0; // Character experience
    bool is_hardcore = false;
    bool is_deleted = false;
    std::string created_at;
    std::string updated_at;
    int gender = 0;      // 0 = male, 1 = female
    int flags = 0;       // Various character flags
    std::string last_played;
};

// Item data structure
struct ItemData {
    int64_t id = 0;
    int64_t owner_id = 0;  // Character ID
    int item_id = 0;       // Game item ID
    int quality = 0;       // Item quality
    int equipped_slot = -1; // -1 = not equipped, 0+ = equipped slot
    std::string attributes; // JSON string of item attributes
    std::string created_at;
    bool is_identified = true;
    int stack_count = 1;
    int enchant_level = 0;
};

// Game session data
struct GameSessionData {
    int id = 0;
    std::string session_name;
    int difficulty = 0;
    int max_players = 4;
    bool is_public = false;
    std::string created_at;
    int created_by_account_id = 0;
    std::string game_mode; // Story, Adventure, etc.
    int act_id = 0;
};

// Friend relationship
struct FriendData {
    int id = 0;
    int account_id = 0;
    int friend_account_id = 0;
    std::string created_at;
    bool is_favorite = false;
};

// Clan data
struct ClanData {
    int id = 0;
    std::string name;
    std::string description;
    int leader_account_id = 0;
    std::string created_at;
    int member_count = 0;
    std::string tag;
    int level = 1;
};

class DatabaseManager {
public:
    DatabaseManager(std::shared_ptr<d3server::core::Config> config);
    ~DatabaseManager();

    bool init();
    bool createTables();

    // Account Management
    bool accountExists(const std::string& login);
    bool verifyAccountPassword(const std::string& login, const std::string& password);
    bool createAccount(const std::string& login, const std::string& email, const std::string& password, const std::string& battleTag);
    bool isAccountBanned(const std::string& login);
    bool updateAccountLastLogin(const std::string& login);
    AccountData getAccountByLogin(const std::string& login);
    bool setAccountBanStatus(const std::string& login, bool banned);
    bool changeAccountPassword(const std::string& login, const std::string& new_password);
    bool deleteAccount(const std::string& login);

    // Character Management
    bool createCharacter(int account_id, const std::string& name, int class_id, int gender, bool hardcore);
    std::vector<CharacterData> getCharactersForAccount(int account_id);
    CharacterData getCharacterById(int64_t character_id);
    bool deleteCharacter(int64_t character_id);
    bool updateCharacterData(const CharacterData& data);
    bool updateCharacterExperience(int64_t character_id, int64_t experience);
    bool updateCharacterLevel(int64_t character_id, int level);

    // Item Management
    int64_t createItem(int64_t character_id, int item_id, int quality, const std::string& attributes);
    std::vector<ItemData> getItemsForCharacter(int64_t character_id);
    bool updateItemData(const ItemData& data);
    bool deleteItem(int64_t item_id);
    bool equipItem(int64_t item_id, int slot);
    bool unequipItem(int64_t item_id);

    // Game Session Management
    int createGameSession(const std::string& name, int difficulty, bool is_public, int created_by_account_id, const std::string& game_mode);
    std::vector<GameSessionData> getPublicGameSessions();
    bool deleteGameSession(int session_id);
    bool updateGameSession(const GameSessionData& data);

    // Friend System
    bool addFriend(int account_id, int friend_account_id);
    bool removeFriend(int account_id, int friend_account_id);
    std::vector<FriendData> getFriendsForAccount(int account_id);
    bool setFriendFavorite(int account_id, int friend_account_id, bool is_favorite);

    // Clan System
    int createClan(const std::string& name, const std::string& description, int leader_account_id, const std::string& tag);
    bool deleteClan(int clan_id);
    bool addClanMember(int clan_id, int account_id);
    bool removeClanMember(int clan_id, int account_id);
    ClanData getClanById(int clan_id);
    std::vector<ClanData> searchClans(const std::string& search_term);

private:
    std::shared_ptr<d3server::core::Config> m_config;
    
    // If using sqlite_orm, declare storage here
    // std::unique_ptr<sqlite_orm::storage<AccountData, CharacterData, ItemData, GameSessionData, FriendData, ClanData>> m_storage;
    
    // For now, using SQLite directly
    void* m_db = nullptr; // sqlite3* pointer
    
    // Helper methods
    bool executeQuery(const std::string& query, const std::vector<std::pair<std::string, std::string>>& params = {});
    std::string generateSalt();
    std::string hashPassword(const std::string& password, const std::string& salt);
    std::string getCurrentTimestamp();
};

} // namespace database_utils
} // namespace d3server

#endif // D3SERVER_DATABASE_MANAGER_H 
