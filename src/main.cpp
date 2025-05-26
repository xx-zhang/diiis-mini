#include <iostream>
#include <string>
#include <memory>
#include <csignal>
#include <thread>

#include "core/server.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"
#include "battle_net/battle_net_server.h"
#include "game_server/game_server.h"
#include "rest_api/rest_server.h"

// Global server instance for signal handling
std::shared_ptr<d3server::core::Server> g_server;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    d3server::core::Logger::getInstance().log(d3server::core::LogLevel::INFO, 
        "Received signal " + std::to_string(signal) + ", shutting down...");
    
    if (g_server) {
        g_server->shutdown();
    }
}

int main(int argc, char* argv[]) {
    // Set up signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // Initialize logger
        auto& logger = d3server::core::Logger::getInstance();
        logger.init();
        logger.log(d3server::core::LogLevel::INFO, "D3Server starting up...");
        
        DEBUG_LOG("Debug mode is enabled");
        
        // Load configuration
        auto config = std::make_shared<d3server::core::Config>();
        config->loadFromFile("config.ini");
        logger.log(d3server::core::LogLevel::INFO, "Configuration loaded");
        
        // Initialize database
        auto dbManager = std::make_shared<d3server::database::DatabaseManager>();
        dbManager->init(config->getDatabaseConfig());
        logger.log(d3server::core::LogLevel::INFO, "Database initialized");
        
        // Initialize server components
        auto battleNetServer = std::make_shared<d3server::battle_net::BattleNetServer>(config, dbManager);
        auto gameServer = std::make_shared<d3server::game_server::GameServer>(config, dbManager);
        auto restServer = std::make_shared<d3server::rest_api::RestServer>(config, dbManager);
        
        // Create main server instance
        g_server = std::make_shared<d3server::core::Server>(
            config, 
            dbManager, 
            battleNetServer,
            gameServer,
            restServer
        );
        
        // Initialize and start the server
        g_server->init();
        g_server->run();
        
        logger.log(d3server::core::LogLevel::INFO, "Server shutdown complete");
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
} 