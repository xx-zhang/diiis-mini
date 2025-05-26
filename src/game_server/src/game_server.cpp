#include "game_server/game_server.h"
#include "game_server/game_client.h"
#include "game_server/game_session.h"
#include "d3core/config.h"
#include "d3core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"

#include <chrono>
#include <algorithm>
#include <functional>
#include <thread>

namespace d3server {
namespace game_server {

GameServer::GameServer(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> db_manager)
    : m_config(config),
      m_dbManager(db_manager),
      m_isRunning(false),
      m_shutdownSignal(false) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_FUNCTION_EXIT();
}

GameServer::~GameServer() {
    DEBUG_FUNCTION_ENTER();
    if (m_isRunning) {
        shutdown();
    }
    DEBUG_FUNCTION_EXIT();
}

bool GameServer::init() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Initializing Game Server...");
    // Load game-specific configurations if any
    // Example: core::GameServerConfig gameCfg = m_config->getGameServerConfig();
    // Initialize worlds, load static game data, etc.

    // For now, just mark as initialized
    m_isRunning = true; // Set to true once init is complete and ready to run
    LOG_INFO("Game Server initialized.");
    DEBUG_FUNCTION_EXIT();
    return true;
}

void GameServer::run() {
    DEBUG_FUNCTION_ENTER();
    if (!m_isRunning) {
        LOG_ERROR("Game Server cannot run, not initialized or initialization failed.");
        DEBUG_FUNCTION_EXIT();
        return;
    }

    LOG_INFO("Starting Game Server run loop...");
    m_gameThread = std::thread(&GameServer::gameLoop, this);
    LOG_INFO("Game Server game loop thread started.");
    DEBUG_FUNCTION_EXIT();
}

void GameServer::shutdown() {
    DEBUG_FUNCTION_ENTER();
    if (!m_isRunning && !m_shutdownSignal) { // Check m_shutdownSignal too in case shutdown is called multiple times
        LOG_INFO("Game Server already shut down or not running.");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down Game Server...");
    m_shutdownSignal.store(true);

    if (m_gameThread.joinable()) {
        LOG_INFO("Waiting for Game Server thread to join...");
        try {
            m_gameThread.join();
            LOG_INFO("Game Server thread joined.");
        } catch (const std::system_error& e) {
            LOG_ERROR("System error while joining game thread: " + std::string(e.what()));
        }
    }
    m_isRunning = false; // Mark as not running after thread has joined
    LOG_INFO("Game Server shut down complete.");
    DEBUG_FUNCTION_EXIT();
}

void GameServer::gameLoop() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Game Server loop started.");

    // Example game loop timing
    const int TICKS_PER_SECOND = 20; // Example: 20 ticks per second
    const auto MS_PER_TICK = std::chrono::milliseconds(1000 / TICKS_PER_SECOND);
    auto lastTickTime = std::chrono::steady_clock::now();

    while (!m_shutdownSignal.load()) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTickTime);

        if (elapsed >= MS_PER_TICK) {
            lastTickTime = now;

            // --- Main Game Logic Tick ---
            // DEBUG_LOG("Game tick...");
            // 1. Process player inputs (if GameServer handles them directly or via queues)
            // 2. Update AI for monsters/NPCs
            // 3. Update world states (projectiles, physics, timed events)
            // 4. Send updates to clients
            // 5. Perform periodic tasks (e.g., saving player data)
            // ----------------------------

            // For now, just a log message for the tick
            // LOG_TICK("Game tick processed."); // Would need a LOG_TICK macro
        }

        // Sleep to avoid busy-waiting and yield CPU
        // Calculate sleep time to aim for the next tick accurately.
        auto nextTickTime = lastTickTime + MS_PER_TICK;
        auto sleepDuration = std::chrono::duration_cast<std::chrono::milliseconds>(nextTickTime - std::chrono::steady_clock::now());
        
        if (sleepDuration.count() > 0) {
            std::this_thread::sleep_for(sleepDuration);
        }
    }
    LOG_INFO("Game Server loop ended.");
    DEBUG_FUNCTION_EXIT();
}

// Placeholder implementations for Player/World management if needed later
// void GameServer::playerConnected(std::shared_ptr<Player> player, uint32_t battleNetSessionId) { ... }
// void GameServer::playerDisconnected(std::shared_ptr<Player> player) { ... }
// std::shared_ptr<Player> GameServer::findPlayerByAccountId(uint32_t accountId) { return nullptr; }
// std::shared_ptr<World> GameServer::createWorld(const std::string& worldName, const std::string& sceneName) { return nullptr; }
// std::shared_ptr<World> GameServer::getWorld(const std::string& worldName) { return nullptr; }

} // namespace game_server
} // namespace d3server 