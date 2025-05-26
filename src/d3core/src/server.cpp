#include "d3core/server.h"
#include "d3core/config.h"
#include "d3core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"
#include "battle_net/battle_net_server.h"
#include "game_server/game_server.h"
#include "rest_api/rest_server.h"

#include <chrono>
#include <thread>

namespace d3server {
namespace d3core {

Server::Server(
    std::shared_ptr<Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager,
    std::shared_ptr<battle_net::BattleNetServer> battleNetServer,
    std::shared_ptr<game_server::GameServer> gameServer,
    std::shared_ptr<rest_api::RestServer> restServer
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_battleNetServer(battleNetServer),
    m_gameServer(gameServer),
    m_restServer(restServer),
    m_running(false)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Server instance created");
    DEBUG_FUNCTION_EXIT();
}

Server::~Server() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("Server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool Server::init() {
    DEBUG_FUNCTION_ENTER();
    
    LOG_INFO("Initializing server...");
    
    // Initialize all components
    try {
        // Initialize Battle.net server
        DEBUG_CONTEXT("Initializing Battle.net server");
        if (!m_battleNetServer->init()) {
            LOG_ERROR("Failed to initialize Battle.net server");
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Initialize game server
        DEBUG_CONTEXT("Initializing game server");
        if (!m_gameServer->init()) {
            LOG_ERROR("Failed to initialize game server");
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Initialize REST API server
        DEBUG_CONTEXT("Initializing REST API server");
        if (!m_restServer->init()) {
            LOG_ERROR("Failed to initialize REST API server");
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        LOG_INFO("Server initialized successfully");
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Exception during server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void Server::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("Server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    LOG_INFO("Starting server components...");
    
    try {
        // Start Battle.net server in a separate thread
        m_serverThreads.emplace_back([this]() {
            try {
                DEBUG_CONTEXT("Starting Battle.net server thread");
                m_battleNetServer->run();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception in Battle.net server thread: " + std::string(e.what()));
            }
        });
        
        // Start game server in a separate thread
        m_serverThreads.emplace_back([this]() {
            try {
                DEBUG_CONTEXT("Starting game server thread");
                m_gameServer->run();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception in game server thread: " + std::string(e.what()));
            }
        });
        
        // Start REST API server in a separate thread
        m_serverThreads.emplace_back([this]() {
            try {
                DEBUG_CONTEXT("Starting REST API server thread");
                m_restServer->run();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception in REST API server thread: " + std::string(e.what()));
            }
        });
        
        LOG_INFO("All server components started");
        
        // Run main server loop
        mainLoop();
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Exception during server startup: " + std::string(e.what()));
        shutdown();
    }
    
    DEBUG_FUNCTION_EXIT();
}

void Server::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("Server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down server...");
    m_running = false;
    
    // Shutdown server components
    try {
        // Shutdown REST API server
        DEBUG_CONTEXT("Shutting down REST API server");
        m_restServer->shutdown();
        
        // Shutdown game server
        DEBUG_CONTEXT("Shutting down game server");
        m_gameServer->shutdown();
        
        // Shutdown Battle.net server
        DEBUG_CONTEXT("Shutting down Battle.net server");
        m_battleNetServer->shutdown();
        
        // Wait for all threads to finish
        for (auto& thread : m_serverThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        m_serverThreads.clear();
        
        LOG_INFO("Server shutdown complete");
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during server shutdown: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

bool Server::isRunning() const {
    return m_running;
}

void Server::mainLoop() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Entering main server loop");
    
    while (m_running) {
        // Perform periodic server tasks here
        
        // Sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    LOG_INFO("Exiting main server loop");
    DEBUG_FUNCTION_EXIT();
}

} // namespace d3core
} // namespace d3server 