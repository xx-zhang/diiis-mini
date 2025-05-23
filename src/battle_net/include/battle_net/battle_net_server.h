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

namespace battle_net {

/**
 * @brief Represents a client connection to the Battle.net server
 */
class BattleNetClient;

/**
 * @brief Battle.net server that handles authentication and game coordination
 */
class BattleNetServer {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    BattleNetServer(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~BattleNetServer();
    
    /**
     * @brief Initialize the Battle.net server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();
    
    /**
     * @brief Run the Battle.net server (blocking call)
     */
    void run();
    
    /**
     * @brief Shutdown the Battle.net server
     */
    void shutdown();
    
    /**
     * @brief Get the number of connected clients
     * @return Number of connected clients
     */
    size_t getClientCount() const;
    
    /**
     * @brief Is the server running
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
     */
    void handleAccept(boost::asio::ip::tcp::socket socket);
    
    /**
     * @brief Process protocol commands
     */
    void processCommands();
    
    /**
     * @brief Remove disconnected clients
     */
    void cleanupClients();
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    std::atomic<bool> m_running;
    std::thread m_ioThread;
    
    mutable std::mutex m_clientsMutex;
    std::unordered_map<std::string, std::shared_ptr<BattleNetClient>> m_clients;
};

} // namespace battle_net
} // namespace d3server 