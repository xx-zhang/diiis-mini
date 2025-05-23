#pragma once

#include <memory>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace d3server {

// Forward declarations
namespace database {
class DatabaseManager;
}

namespace battle_net {
class BattleNetServer;
}

namespace game_server {
class GameServer;
}

namespace rest_api {
class RestServer;
}

namespace core {

class Config;

/**
 * @brief Main server class that coordinates all server components
 */
class Server {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     * @param battleNetServer Battle.net server instance
     * @param gameServer Game server instance
     * @param restServer REST API server instance
     */
    Server(
        std::shared_ptr<Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager,
        std::shared_ptr<battle_net::BattleNetServer> battleNetServer,
        std::shared_ptr<game_server::GameServer> gameServer,
        std::shared_ptr<rest_api::RestServer> restServer
    );

    /**
     * @brief Destructor
     */
    ~Server();

    /**
     * @brief Initialize the server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();

    /**
     * @brief Run the server (blocking call)
     */
    void run();

    /**
     * @brief Shutdown the server
     */
    void shutdown();

    /**
     * @brief Check if the server is running
     * @return True if the server is running, false otherwise
     */
    bool isRunning() const;

private:
    std::shared_ptr<Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<battle_net::BattleNetServer> m_battleNetServer;
    std::shared_ptr<game_server::GameServer> m_gameServer;
    std::shared_ptr<rest_api::RestServer> m_restServer;

    std::atomic<bool> m_running;
    std::vector<std::thread> m_serverThreads;

    /**
     * @brief Main server loop
     */
    void mainLoop();
};

} // namespace core
} // namespace d3server 