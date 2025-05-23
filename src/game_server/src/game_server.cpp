#include "game_server/game_server.h"
#include "game_server/game_client.h"
#include "game_server/game_session.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"

#include <chrono>
#include <algorithm>
#include <functional>

namespace d3server {
namespace game_server {

GameServer::GameServer(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_ioContext(),
    m_acceptor(m_ioContext),
    m_running(false),
    m_nextSessionId(1)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Game server instance created");
    DEBUG_FUNCTION_EXIT();
}

GameServer::~GameServer() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("Game server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool GameServer::init() {
    DEBUG_FUNCTION_ENTER();
    
    try {
        LOG_INFO("Initializing game server...");
        
        // Initialize acceptor
        auto& networkConfig = m_config->getNetworkConfig();
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(networkConfig.bindIp),
            networkConfig.gameServerPort
        );
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
        
        LOG_INFO("Game server initialized on " + networkConfig.bindIp + ":" + std::to_string(networkConfig.gameServerPort));
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during game server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void GameServer::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("Game server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    
    LOG_INFO("Starting game server...");
    
    // Start accepting connections
    startAccept();
    
    // Run the IO context in a separate thread
    m_ioThread = std::thread([this]() {
        DEBUG_CONTEXT("Game server IO thread started");
        
        try {
            m_ioContext.run();
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception in game server IO thread: " + std::string(e.what()));
        }
        
        DEBUG_LOG("Game server IO thread exited");
    });
    
    // Run the update loop in a separate thread
    m_updateThread = std::thread([this]() {
        DEBUG_CONTEXT("Game server update thread started");
        
        auto lastUpdateTime = std::chrono::steady_clock::now();
        
        while (m_running) {
            // Calculate delta time
            auto currentTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
            lastUpdateTime = currentTime;
            
            // Update sessions
            updateSessions();
            
            // Clean up disconnected clients and empty sessions
            cleanupClientsAndSessions();
            
            // Sleep to avoid high CPU usage (aim for 60 updates per second)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        DEBUG_LOG("Game server update thread exited");
    });
    
    LOG_INFO("Game server started");
    DEBUG_FUNCTION_EXIT();
}

void GameServer::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("Game server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down game server...");
    m_running = false;
    
    // Stop the IO context
    m_ioContext.stop();
    
    // Wait for threads to finish
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
    
    if (m_updateThread.joinable()) {
        m_updateThread.join();
    }
    
    // Disconnect all clients
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.clear();
    }
    
    // Clear all sessions
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        m_sessions.clear();
    }
    
    LOG_INFO("Game server shutdown complete");
    DEBUG_FUNCTION_EXIT();
}

size_t GameServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    return m_clients.size();
}

size_t GameServer::getSessionCount() const {
    std::lock_guard<std::mutex> lock(m_sessionsMutex);
    return m_sessions.size();
}

bool GameServer::isRunning() const {
    return m_running;
}

void GameServer::startAccept() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        DEBUG_LOG("Game server is not running, not accepting connections");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                // Handle the new connection
                handleAccept(std::move(socket));
            } else {
                DEBUG_LOG("Error accepting connection: " + ec.message());
            }
            
            // Accept the next connection
            startAccept();
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void GameServer::handleAccept(boost::asio::ip::tcp::socket socket) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        // Get client IP address
        std::string clientIp = socket.remote_endpoint().address().to_string();
        LOG_INFO("New game connection from " + clientIp);
        
        // Create a new client session
        auto client = std::make_shared<GameClient>(
            std::move(socket),
            *this,
            m_config,
            m_dbManager
        );
        
        // Add client to the list
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clients[client->getIpAddress()] = client;
        }
        
        // Start processing client
        client->start();
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception handling new game connection: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

void GameServer::updateSessions() {
    // Calculate delta time based on elapsed time since last update
    static auto lastUpdateTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
    lastUpdateTime = currentTime;
    
    // Update each session
    std::lock_guard<std::mutex> lock(m_sessionsMutex);
    for (auto& sessionPair : m_sessions) {
        sessionPair.second->update(deltaTime);
    }
}

void GameServer::cleanupClientsAndSessions() {
    DEBUG_FUNCTION_ENTER();
    
    // Clean up disconnected clients
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        
        std::vector<std::string> disconnectedClients;
        for (const auto& clientPair : m_clients) {
            if (!clientPair.second->isConnected()) {
                disconnectedClients.push_back(clientPair.first);
            }
        }
        
        for (const auto& ip : disconnectedClients) {
            LOG_INFO("Removing disconnected game client: " + ip);
            m_clients.erase(ip);
        }
    }
    
    // Clean up empty sessions
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        
        std::vector<uint32_t> emptySessions;
        for (const auto& sessionPair : m_sessions) {
            if (sessionPair.second->isEmpty()) {
                emptySessions.push_back(sessionPair.first);
            }
        }
        
        for (uint32_t sessionId : emptySessions) {
            LOG_INFO("Removing empty game session: " + std::to_string(sessionId));
            m_sessions.erase(sessionId);
        }
    }
    
    DEBUG_FUNCTION_EXIT();
}

std::shared_ptr<GameSession> GameServer::createGameSession() {
    DEBUG_FUNCTION_ENTER();
    
    // Generate a unique session ID
    uint32_t sessionId = m_nextSessionId++;
    
    // Create the session
    auto session = std::make_shared<GameSession>(sessionId, m_config, m_dbManager);
    
    // Initialize the session
    bool success = session->init();
    if (!success) {
        LOG_ERROR("Failed to initialize game session: " + std::to_string(sessionId));
        DEBUG_FUNCTION_EXIT();
        return nullptr;
    }
    
    // Add to the sessions map
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        m_sessions[sessionId] = session;
    }
    
    LOG_INFO("Created game session: " + std::to_string(sessionId));
    DEBUG_FUNCTION_EXIT();
    return session;
}

std::shared_ptr<GameSession> GameServer::getGameSession(uint32_t sessionId) {
    std::lock_guard<std::mutex> lock(m_sessionsMutex);
    
    auto it = m_sessions.find(sessionId);
    if (it != m_sessions.end()) {
        return it->second;
    }
    
    return nullptr;
}

} // namespace game_server
} // namespace d3server 