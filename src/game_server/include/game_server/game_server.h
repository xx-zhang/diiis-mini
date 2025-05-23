#pragma once

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace game_server {

/**
 * @brief Represents a connected game client
 */
class GameClient;

/**
 * @brief Represents a game session
 */
class GameSession;

/**
 * @brief Game server that handles in-game communication and game logic
 */
class GameServer {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    GameServer(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~GameServer();
    
    /**
     * @brief Initialize the game server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();
    
    /**
     * @brief Run the game server (blocking call)
     */
    void run();
    
    /**
     * @brief Shutdown the game server
     */
    void shutdown();
    
    /**
     * @brief Get the number of connected clients
     * @return Number of connected clients
     */
    size_t getClientCount() const;
    
    /**
     * @brief Get the number of active game sessions
     * @return Number of active game sessions
     */
    size_t getSessionCount() const;
    
    /**
     * @brief Check if the server is running
     * @return True if the server is running, false otherwise
     */
    bool isRunning() const;
    
private:
    /**
     * @brief Start accepting connections
     */
    void startAccept();
    
    /**
     * @brief Handle a new connection
     * @param socket Socket for the new connection
     */
    void handleAccept(boost::asio::ip::tcp::socket socket);
    
    /**
     * @brief Update game sessions
     */
    void updateSessions();
    
    /**
     * @brief Clean up disconnected clients and empty sessions
     */
    void cleanupClientsAndSessions();
    
    /**
     * @brief Create a new game session
     * @return Shared pointer to the new game session
     */
    std::shared_ptr<GameSession> createGameSession();
    
    /**
     * @brief Get a game session by ID
     * @param sessionId Session ID
     * @return Shared pointer to the game session, or nullptr if not found
     */
    std::shared_ptr<GameSession> getGameSession(uint32_t sessionId);
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    std::atomic<bool> m_running;
    std::atomic<uint32_t> m_nextSessionId;
    std::thread m_ioThread;
    std::thread m_updateThread;
    
    mutable std::mutex m_clientsMutex;
    std::unordered_map<std::string, std::shared_ptr<GameClient>> m_clients;
    
    mutable std::mutex m_sessionsMutex;
    std::unordered_map<uint32_t, std::shared_ptr<GameSession>> m_sessions;
};

} // namespace game_server
} // namespace d3server 