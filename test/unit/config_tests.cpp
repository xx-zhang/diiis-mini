#include <gtest/gtest.h>
#include "core/config.h"
#include <memory>
#include <string>
#include <filesystem>

namespace d3server {
namespace core {
namespace test {

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test config database in memory
        m_config = std::make_shared<Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
    }
    
    void TearDown() override {
        m_config.reset();
    }
    
    std::shared_ptr<Config> m_config;
};

// Test server configuration
TEST_F(ConfigTest, ServerConfig) {
    // Get server config
    auto serverConfig = m_config->getServerConfig();
    
    // Test default values
    EXPECT_FALSE(serverConfig.serverName.empty());
    EXPECT_GE(serverConfig.maxPlayers, 1);
    EXPECT_GE(serverConfig.maxCharactersPerAccount, 1);
    
    // Test setting and getting values
    serverConfig.serverName = "TestServer";
    serverConfig.maxPlayers = 100;
    serverConfig.maxCharactersPerAccount = 10;
    serverConfig.enableDebug = true;
    
    // Update config
    EXPECT_TRUE(m_config->updateServerConfig(serverConfig));
    
    // Get updated config
    auto updatedConfig = m_config->getServerConfig();
    
    // Verify values were saved
    EXPECT_EQ(updatedConfig.serverName, "TestServer");
    EXPECT_EQ(updatedConfig.maxPlayers, 100);
    EXPECT_EQ(updatedConfig.maxCharactersPerAccount, 10);
    EXPECT_TRUE(updatedConfig.enableDebug);
}

// Test network configuration
TEST_F(ConfigTest, NetworkConfig) {
    // Get network config
    auto networkConfig = m_config->getNetworkConfig();
    
    // Test default values
    EXPECT_FALSE(networkConfig.bindIp.empty());
    EXPECT_GT(networkConfig.restApiPort, 0);
    EXPECT_GT(networkConfig.battleNetPort, 0);
    EXPECT_GT(networkConfig.gameServerPort, 0);
    
    // Test setting and getting values
    networkConfig.bindIp = "127.0.0.1";
    networkConfig.restApiPort = 8080;
    networkConfig.battleNetPort = 1119;
    networkConfig.gameServerPort = 1120;
    
    // Update config
    EXPECT_TRUE(m_config->updateNetworkConfig(networkConfig));
    
    // Get updated config
    auto updatedConfig = m_config->getNetworkConfig();
    
    // Verify values were saved
    EXPECT_EQ(updatedConfig.bindIp, "127.0.0.1");
    EXPECT_EQ(updatedConfig.restApiPort, 8080);
    EXPECT_EQ(updatedConfig.battleNetPort, 1119);
    EXPECT_EQ(updatedConfig.gameServerPort, 1120);
}

// Test database configuration
TEST_F(ConfigTest, DatabaseConfig) {
    // Get database config
    auto dbConfig = m_config->getDatabaseConfig();
    
    // Test default values
    EXPECT_FALSE(dbConfig.dbPath.empty());
    
    // Test setting and getting values
    dbConfig.dbPath = "test_db.sqlite";
    
    // Update config
    EXPECT_TRUE(m_config->updateDatabaseConfig(dbConfig));
    
    // Get updated config
    auto updatedConfig = m_config->getDatabaseConfig();
    
    // Verify values were saved
    EXPECT_EQ(updatedConfig.dbPath, "test_db.sqlite");
}

// Test config file loading and saving
TEST_F(ConfigTest, ConfigFileSaveLoad) {
    // Create temporary file path
    std::string tempConfigPath = "temp_config_test.db";
    
    // Create new config instance
    auto configToSave = std::make_shared<Config>();
    ASSERT_TRUE(configToSave->init(tempConfigPath));
    
    // Set some values
    auto serverConfig = configToSave->getServerConfig();
    serverConfig.serverName = "SaveLoadTest";
    serverConfig.maxPlayers = 200;
    EXPECT_TRUE(configToSave->updateServerConfig(serverConfig));
    
    // Close the config
    configToSave.reset();
    
    // Load the config back
    auto loadedConfig = std::make_shared<Config>();
    ASSERT_TRUE(loadedConfig->init(tempConfigPath));
    
    // Verify values were loaded
    auto loadedServerConfig = loadedConfig->getServerConfig();
    EXPECT_EQ(loadedServerConfig.serverName, "SaveLoadTest");
    EXPECT_EQ(loadedServerConfig.maxPlayers, 200);
    
    // Clean up
    loadedConfig.reset();
    std::filesystem::remove(tempConfigPath);
}

} // namespace test
} // namespace core
} // namespace d3server 