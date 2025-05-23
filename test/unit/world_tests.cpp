#include <gtest/gtest.h>
#include "game_server/game_session.h"
#include "core/config.h"
#include "database/database_manager.h"
#include <memory>

namespace d3server {
namespace game_server {
namespace test {

class WorldTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
};

// Test different game modes
TEST_F(WorldTest, GameModes) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Test Campaign mode
    session->setGameMode(GameMode::Campaign);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Campaign));
    
    // Test Adventure mode
    session->setGameMode(GameMode::Adventure);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Adventure));
    
    // Test Rift mode
    session->setGameMode(GameMode::Rift);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::Rift));
    
    // Test Greater Rift mode
    session->setGameMode(GameMode::GreaterRift);
    EXPECT_EQ(static_cast<int>(session->getGameMode()), static_cast<int>(GameMode::GreaterRift));
}

// Test difficulty levels
TEST_F(WorldTest, DifficultyLevels) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Test Normal difficulty
    session->setDifficulty(GameDifficulty::Normal);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Normal));
    
    // Test Hard difficulty
    session->setDifficulty(GameDifficulty::Hard);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Hard));
    
    // Test Expert difficulty
    session->setDifficulty(GameDifficulty::Expert);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Expert));
    
    // Test Master difficulty
    session->setDifficulty(GameDifficulty::Master);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Master));
    
    // Test Torment difficulties
    session->setDifficulty(GameDifficulty::Torment1);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Torment1));
    
    session->setDifficulty(GameDifficulty::Torment6);
    EXPECT_EQ(static_cast<int>(session->getDifficulty()), static_cast<int>(GameDifficulty::Torment6));
}

// Test multiple game sessions
TEST_F(WorldTest, MultipleSessions) {
    // Create multiple game sessions
    auto session1 = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session1->init());
    
    auto session2 = std::make_shared<GameSession>(2, m_config, m_dbManager);
    ASSERT_TRUE(session2->init());
    
    auto session3 = std::make_shared<GameSession>(3, m_config, m_dbManager);
    ASSERT_TRUE(session3->init());
    
    // Each session should have a unique ID
    EXPECT_EQ(session1->getSessionId(), 1);
    EXPECT_EQ(session2->getSessionId(), 2);
    EXPECT_EQ(session3->getSessionId(), 3);
    
    // Set different modes and difficulties
    session1->setGameMode(GameMode::Campaign);
    session1->setDifficulty(GameDifficulty::Normal);
    
    session2->setGameMode(GameMode::Adventure);
    session2->setDifficulty(GameDifficulty::Hard);
    
    session3->setGameMode(GameMode::Rift);
    session3->setDifficulty(GameDifficulty::Torment1);
    
    // Each session should maintain its own state
    EXPECT_EQ(static_cast<int>(session1->getGameMode()), static_cast<int>(GameMode::Campaign));
    EXPECT_EQ(static_cast<int>(session1->getDifficulty()), static_cast<int>(GameDifficulty::Normal));
    
    EXPECT_EQ(static_cast<int>(session2->getGameMode()), static_cast<int>(GameMode::Adventure));
    EXPECT_EQ(static_cast<int>(session2->getDifficulty()), static_cast<int>(GameDifficulty::Hard));
    
    EXPECT_EQ(static_cast<int>(session3->getGameMode()), static_cast<int>(GameMode::Rift));
    EXPECT_EQ(static_cast<int>(session3->getDifficulty()), static_cast<int>(GameDifficulty::Torment1));
}

// Test session updates
TEST_F(WorldTest, SessionUpdates) {
    // Create a game session
    auto session = std::make_shared<GameSession>(1, m_config, m_dbManager);
    ASSERT_TRUE(session->init());
    
    // Check that session can be updated multiple times
    for (int i = 0; i < 10; i++) {
        session->update(0.016f); // 16ms frame time
    }
    
    // Session should still be in a valid state
    EXPECT_TRUE(session->isEmpty());
    EXPECT_EQ(session->getClientCount(), 0);
}

} // namespace test
} // namespace game_server
} // namespace d3server 