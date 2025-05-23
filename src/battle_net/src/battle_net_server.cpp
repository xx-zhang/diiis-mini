#include "battle_net/battle_net_server.h"
#include "battle_net/battle_net_client.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"

#include <functional>
#include <algorithm>
#include <chrono>

namespace d3server {
namespace battle_net {

BattleNetServer::BattleNetServer(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_ioContext(),
    m_acceptor(m_ioContext),
    m_running(false)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Battle.net server instance created");
    DEBUG_FUNCTION_EXIT();
}

BattleNetServer::~BattleNetServer() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("Battle.net server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool BattleNetServer::init() {
    DEBUG_FUNCTION_ENTER();
    
    try {
        LOG_INFO("Initializing Battle.net server...");
        
        // Initialize acceptor
        auto& networkConfig = m_config->getNetworkConfig();
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(networkConfig.bindIp),
            networkConfig.battleNetPort
        );
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
        
        LOG_INFO("Battle.net server initialized on " + networkConfig.bindIp + ":" + std::to_string(networkConfig.battleNetPort));
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during Battle.net server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void BattleNetServer::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("Battle.net server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    
    LOG_INFO("Starting Battle.net server...");
    
    // Start accepting connections
    startAccept();
    
    // Run the IO context
    m_ioThread = std::thread([this]() {
        DEBUG_CONTEXT("Battle.net server IO thread started");
        
        try {
            m_ioContext.run();
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception in Battle.net server IO thread: " + std::string(e.what()));
        }
        
        DEBUG_LOG("Battle.net server IO thread exited");
    });
    
    LOG_INFO("Battle.net server started");
    
    // Run the command processing loop
    while (m_running) {
        // Process commands from clients
        processCommands();
        
        // Clean up disconnected clients
        cleanupClients();
        
        // Sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetServer::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("Battle.net server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down Battle.net server...");
    m_running = false;
    
    // Stop the IO context
    m_ioContext.stop();
    
    // Wait for the IO thread to finish
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
    
    // Disconnect all clients
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.clear();
    }
    
    LOG_INFO("Battle.net server shutdown complete");
    DEBUG_FUNCTION_EXIT();
}

size_t BattleNetServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    return m_clients.size();
}

bool BattleNetServer::isRunning() const {
    return m_running;
}

void BattleNetServer::startAccept() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        DEBUG_LOG("Battle.net server is not running, not accepting connections");
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

void BattleNetServer::handleAccept(boost::asio::ip::tcp::socket socket) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        // Get client IP address
        std::string clientIp = socket.remote_endpoint().address().to_string();
        LOG_INFO("New connection from " + clientIp);
        
        // Check if client IP is already connected too many times
        size_t connectionCount = 0;
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (const auto& client : m_clients) {
                if (client.second->getIpAddress() == clientIp) {
                    connectionCount++;
                }
            }
        }
        
        // Check if IP has reached max connections limit
        if (connectionCount >= static_cast<size_t>(m_config->getServerConfig().maxAccountsPerIp)) {
            LOG_WARNING("Connection from " + clientIp + " rejected (too many connections)");
            socket.close();
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Create a new client session
        auto client = std::make_shared<BattleNetClient>(
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
        LOG_ERROR("Exception handling new connection: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetServer::processCommands() {
    // This would process any pending commands from clients
    // For simplicity, we're not implementing actual command processing here
}

void BattleNetServer::cleanupClients() {
    DEBUG_FUNCTION_ENTER();
    
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    // Find disconnected clients
    std::vector<std::string> disconnectedClients;
    for (const auto& client : m_clients) {
        if (!client.second->isConnected()) {
            disconnectedClients.push_back(client.first);
        }
    }
    
    // Remove disconnected clients
    for (const auto& ip : disconnectedClients) {
        LOG_INFO("Removing disconnected client: " + ip);
        m_clients.erase(ip);
    }
    
    DEBUG_FUNCTION_EXIT();
}

} // namespace battle_net
} // namespace d3server 