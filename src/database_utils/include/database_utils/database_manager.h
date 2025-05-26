#ifndef D3SERVER_DATABASE_MANAGER_H
#define D3SERVER_DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>

// 前置声明
namespace d3server {
    namespace core {
        class Config;
        class Logger;
    }
    namespace database_utils {
        class CryptoUtils;
    }
}

namespace d3server {
namespace database_utils {

/**
 * @brief 账户数据结构体
 * 
 * 存储游戏账户的详细信息，包括基本认证和状态信息
 */
struct AccountData {
    int id = 0;                   ///< 唯一标识符
    std::string login;            ///< 登录名
    std::string email;            ///< 电子邮件地址
    std::string password_hash;    ///< 密码哈希值
    std::string salt;             ///< 密码加密盐值
    bool is_banned = false;       ///< 账户是否被封禁
    std::string last_login;       ///< 最后登录时间
    std::string created_at;       ///< 账户创建时间
    std::string updated_at;       ///< 账户最后更新时间
    int account_level = 0;        ///< 账户等级（普通、VIP等）
    std::string battle_tag;       ///< 游戏内唯一标识
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

/**
 * @brief 数据库管理器类
 * 
 * 提供数据库操作的核心功能，包括账户管理、数据持久化等
 */
class DatabaseManager {
public:
    /**
     * @brief 构造函数
     * 
     * @param config 配置对象智能指针，用于初始化数据库连接
     */
    explicit DatabaseManager(std::shared_ptr<core::Config> config);

    /**
     * @brief 析构函数
     * 
     * 确保安全关闭数据库连接
     */
    ~DatabaseManager();

    /**
     * @brief 初始化数据库
     * 
     * 建立数据库连接，创建必要的表结构
     * 
     * @return bool 初始化是否成功
     */
    bool init();

    /**
     * @brief 创建新账户
     * 
     * @param login 登录名
     * @param email 电子邮件
     * @param password 明文密码
     * @param battleTag 游戏标签
     * @return bool 账户创建是否成功
     */
    bool createAccount(
        const std::string& login, 
        const std::string& email, 
        const std::string& password, 
        const std::string& battleTag
    );

    /**
     * @brief 验证账户凭据
     * 
     * @param login 登录名
     * @param password 明文密码
     * @return bool 验证是否通过
     */
    bool verifyAccount(
        const std::string& login, 
        const std::string& password
    );

    /**
     * @brief 获取账户详细信息
     * 
     * @param login 登录名
     * @return std::optional<AccountData> 账户数据，如果不存在则返回空
     */
    std::optional<AccountData> getAccountDetails(
        const std::string& login
    );

    // 禁用拷贝构造和赋值运算符
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Account Management
    bool accountExists(const std::string& login);
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
    std::shared_ptr<core::Config> m_config;  ///< 配置对象
    void* m_db = nullptr;  ///< SQLite数据库连接指针

    /**
     * @brief 创建数据库表结构
     * 
     * @return bool 表创建是否成功
     */
    bool createTables();

    /**
     * @brief 生成随机盐值
     * 
     * @return std::string 生成的盐值
     */
    std::string generateSalt();

    /**
     * @brief 密码哈希
     * 
     * @param password 明文密码
     * @param salt 盐值
     * @return std::string 哈希后的密码
     */
    std::string hashPassword(
        const std::string& password, 
        const std::string& salt
    );

    /**
     * @brief Execute a SQL query with optional parameters
     * @param query SQL query string
     * @param params Optional query parameters
     * @return bool True if query execution successful, false otherwise
     */
    bool executeQuery(
        const std::string& query, 
        const std::vector<std::pair<std::string, std::string>>& params = {}
    );

    /**
     * @brief Get current timestamp
     * @return std::string Current timestamp in format YYYY-MM-DD HH:MM:SS
     */
    std::string getCurrentTimestamp();
};

} // namespace database_utils
} // namespace d3server

#endif // D3SERVER_DATABASE_MANAGER_H 
