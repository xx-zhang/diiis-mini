#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <chrono>

namespace d3server {

// Forward declarations
namespace d3core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace game_server {

// Forward declarations
class GameClient;

/**
 * @brief Represents a game difficulty level
 */
enum class GameDifficulty {
    Normal,
    Hard,
    Expert,
    Master,
    Torment1,
    Torment2,
    Torment3,
    Torment4,
    Torment5,
    Torment6
};

/**
 * @brief Represents a game mode
 */
enum class GameMode {
    Campaign,
    Adventure,
    Rift,
    GreaterRift
};

/**
 * @brief Represents a game session where players can join and play together
 */
class GameSession : public std::enable_shared_from_this<GameSession> {
public:
    /**
     * @brief Constructor
     * @param sessionId Unique session ID
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    GameSession(
        uint32_t sessionId,
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~GameSession();
    
    /**
     * @brief Initialize the game session
     * @return True if initialization succeeded, false otherwise
     */
    bool init();
    
    /**
     * @brief Update the game session
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);
    
    /**
     * @brief Add a client to the session
     * @param client Client to add
     * @return True if client was added, false otherwise
     */
    bool addClient(std::shared_ptr<GameClient> client);
    
    /**
     * @brief Remove a client from the session
     * @param client Client to remove
     */
    void removeClient(std::shared_ptr<GameClient> client);
    
    /**
     * @brief Check if the session is empty
     * @return True if the session has no clients, false otherwise
     */
    bool isEmpty() const;
    
    /**
     * @brief Get the session ID
     * @return Session ID
     */
    uint32_t getSessionId() const;
    
    /**
     * @brief Get the number of clients in the session
     * @return Number of clients
     */
    size_t getClientCount() const;
    
    /**
     * @brief Get the difficulty level
     * @return Difficulty level
     */
    GameDifficulty getDifficulty() const;
    
    /**
     * @brief Set the difficulty level
     * @param difficulty Difficulty level
     */
    void setDifficulty(GameDifficulty difficulty);
    
    /**
     * @brief Get the game mode
     * @return Game mode
     */
    GameMode getGameMode() const;
    
    /**
     * @brief Set the game mode
     * @param mode Game mode
     */
    void setGameMode(GameMode mode);
    
    /**
     * @brief Broadcast a message to all clients in the session
     * @param message Message to broadcast
     * @param excludeClient Client to exclude from broadcast (nullptr to send to all)
     */
    void broadcastMessage(const std::vector<uint8_t>& message, std::shared_ptr<GameClient> excludeClient = nullptr);
    
private:
    /**
     * @brief Handle a client message
     * @param client Client that sent the message
     * @param message Message data
     */
    void handleMessage(std::shared_ptr<GameClient> client, const std::vector<uint8_t>& message);
    
    /**
     * @brief Process game logic
     * @param deltaTime Time elapsed since last update
     */
    void processGameLogic(float deltaTime);
    
    uint32_t m_sessionId;
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    std::atomic<GameDifficulty> m_difficulty;
    std::atomic<GameMode> m_mode;
    std::chrono::steady_clock::time_point m_creationTime;
    
    mutable std::mutex m_clientsMutex;
    std::unordered_map<std::string, std::shared_ptr<GameClient>> m_clients;
};

} // namespace game_server
} // namespace d3server 