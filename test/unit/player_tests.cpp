#include <gtest/gtest.h>
#include "game_server/game_client.h"
#include "game_server/game_server.h"
#include "game_server/game_session.h"
#include "core/config.h"
#include "database/database_manager.h"
#include <memory>
#include <vector>
#include <string>

namespace d3server {
namespace game_server {
namespace test {

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Setup test database
        setupTestDatabase();
        
        // Create game server
        m_gameServer = std::make_shared<GameServer>(m_config, m_dbManager);
        ASSERT_TRUE(m_gameServer->init());
    }
    
    void TearDown() override {
        m_gameServer.reset();
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Setup test database with test accounts and characters
    void setupTestDatabase() {
        // Create test accounts
        ASSERT_TRUE(m_dbManager->createAccount("player1", "player1@example.com", "password123"));
        ASSERT_TRUE(m_dbManager->createAccount("player2", "player2@example.com", "password123"));
        
        // Create characters
        database::Character char1;
        char1.name = "Barbarian1";
        char1.classId = 1; // Barbarian
        char1.level = 10;
        ASSERT_TRUE(m_dbManager->createCharacter("player1", char1));
        
        database::Character char2;
        char2.name = "Wizard1";
        char2.classId = 3; // Wizard
        char2.level = 15;
        ASSERT_TRUE(m_dbManager->createCharacter("player2", char2));
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<GameServer> m_gameServer;
};

// Test session difficulty and game mode
TEST_F(PlayerTest, SessionDifficultyAndMode) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Check default difficulty and mode
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Normal));
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Campaign));
    
    // Change difficulty and mode
    session->setDifficulty(GameDifficulty::Torment1);
    session->setGameMode(GameMode::Adventure);
    
    // Check updated difficulty and mode
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Torment1));
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Adventure));
}

// Test empty session
TEST_F(PlayerTest, EmptySession) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Session should be empty initially
    EXPECT_TRUE(session->isEmpty());
    EXPECT_EQ(session->getClientCount(), 0);
    EXPECT_EQ(session->getSessionId(), 1);
}

// Test game server operations
TEST_F(PlayerTest, GameServerOperations) {
    // Game server should be initialized
    EXPECT_TRUE(m_gameServer->isRunning());
    
    // Should have no clients initially
    EXPECT_EQ(m_gameServer->getClientCount(), 0);
    
    // Should have no sessions initially
    EXPECT_EQ(m_gameServer->getSessionCount(), 0);
}

} // namespace test
} // namespace game_server
} // namespace d3server 