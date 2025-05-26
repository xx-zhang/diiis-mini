#ifndef D3SERVER_GAME_SERVER_H
#define D3SERVER_GAME_SERVER_H

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <boost/asio.hpp>
#include "d3core/config.h"
#include "d3core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"

namespace d3server {

// Forward declarations
namespace d3core {
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

// Forward declare Player for now, will be a more complex class
class Player;
class World;

/**
 * @brief Game server that handles in-game communication and game logic
 */
class GameServer : public std::enable_shared_from_this<GameServer> {
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
    
    // Player/Session Management (examples)
    // void playerConnected(std::shared_ptr<Player> player, uint32_t battleNetSessionId);
    // void playerDisconnected(std::shared_ptr<Player> player);
    // std::shared_ptr<Player> findPlayerByAccountId(uint32_t accountId);

    // World Management (examples)
    // std::shared_ptr<World> createWorld(const std::string& worldName, const std::string& sceneName);
    // std::shared_ptr<World> getWorld(const std::string& worldName);

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
    
    void gameLoop(); // The actual loop run in m_gameThread
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    std::thread m_gameThread; 
    bool m_isRunning;
    std::atomic<bool> m_shutdownSignal{false};

    // Example: Managing active players and worlds
    // std::set<std::shared_ptr<Player>> m_activePlayers;
    // std::mutex m_playersMutex;
    // std::vector<std::shared_ptr<World>> m_worlds;
    // std::mutex m_worldsMutex;

    // If GameServer listens for its own connections (e.g., UDP or specific game TCP)
    // boost::asio::io_context m_ioContext;
    // boost::asio::ip::tcp::acceptor m_acceptor; // Or udp::socket
    // std::thread m_ioThread;
};

} // namespace game_server
} // namespace d3server

#endif // D3SERVER_GAME_SERVER_H 