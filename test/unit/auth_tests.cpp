#include <gtest/gtest.h>
#include "battle_net/battle_net_client.h"
#include "battle_net/battle_net_server.h"
#include "core/config.h"
#include "database/database_manager.h"
#include "utils/crypto_utils.h"
#include <memory>
#include <vector>
#include <string>

namespace d3server {
namespace battle_net {
namespace test {

class AuthTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Setup test database
        setupTestDatabase();
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Setup test database with test accounts
    void setupTestDatabase() {
        // Create test accounts
        ASSERT_TRUE(m_dbManager->createAccount("testuser", "test@example.com", "password123"));
        ASSERT_TRUE(m_dbManager->createAccount("banneduser", "banned@example.com", "password123"));
        
        // Ban one of the accounts
        ASSERT_TRUE(m_dbManager->setBanStatus("banneduser", true, "Test ban reason"));
    }
    
    // Helper to create an authentication message
    std::vector<uint8_t> createAuthMessage(const std::string& login, const std::string& password) {
        std::vector<uint8_t> message;
        
        // Message type: Auth (0x01)
        message.push_back(0x01);
        
        // Login length
        message.push_back(static_cast<uint8_t>(login.length()));
        
        // Login
        message.insert(message.end(), login.begin(), login.end());
        
        // Password length
        message.push_back(static_cast<uint8_t>(password.length()));
        
        // Password
        message.insert(message.end(), password.begin(), password.end());
        
        return message;
    }
    
    // Mock client for testing
    class MockBattleNetClient : public BattleNetClient {
    public:
        MockBattleNetClient(BattleNetServer& server, 
                            std::shared_ptr<core::Config> config,
                            std::shared_ptr<database::DatabaseManager> dbManager)
            : BattleNetClient(boost::asio::ip::tcp::socket(m_ioContext), server, config, dbManager),
              m_lastSentMessage(0) {
        }
        
        // Override to capture sent messages
        void sendMessage(const std::vector<uint8_t>& message) override {
            m_lastSentMessage = message;
        }
        
        // Process a message directly
        bool processTestMessage(const std::vector<uint8_t>& message) {
            try {
                processMessage(message);
                return true;
            }
            catch (...) {
                return false;
            }
        }
        
        // Get the last sent message
        std::vector<uint8_t> getLastSentMessage() const {
            return m_lastSentMessage;
        }
        
    private:
        boost::asio::io_context m_ioContext;
        std::vector<uint8_t> m_lastSentMessage;
    };
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
};

// Test successful authentication
TEST_F(AuthTest, SuccessfulAuth) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message
    auto authMessage = createAuthMessage("testuser", "password123");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::Authenticated);
    EXPECT_EQ(client.getLogin(), "testuser");
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x01); // Success
}

// Test failed authentication with wrong password
TEST_F(AuthTest, FailedAuthWrongPassword) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message with wrong password
    auto authMessage = createAuthMessage("testuser", "wrongpassword");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::AuthFailed);
    EXPECT_TRUE(client.getLogin().empty());
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x00); // Failure
}

// Test failed authentication with non-existent user
TEST_F(AuthTest, FailedAuthNonExistentUser) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message with non-existent user
    auto authMessage = createAuthMessage("nonexistent", "password123");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::AuthFailed);
    EXPECT_TRUE(client.getLogin().empty());
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x00); // Failure
}

// Test banned account authentication
TEST_F(AuthTest, FailedAuthBannedAccount) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create auth message for banned account
    auto authMessage = createAuthMessage("banneduser", "password123");
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(authMessage));
    
    // Check authentication state
    EXPECT_EQ(client.getAuthState(), AuthState::AuthFailed);
    EXPECT_TRUE(client.getLogin().empty());
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x02); // Auth response type
    EXPECT_EQ(response[1], 0x00); // Failure
}

// Test malformed authentication message
TEST_F(AuthTest, MalformedAuthMessage) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create malformed auth message (too short)
    std::vector<uint8_t> malformedMessage = {0x01, 0x08};
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(malformedMessage));
    
    // Check authentication state - should still be not authenticated
    EXPECT_EQ(client.getAuthState(), AuthState::NotAuthenticated);
}

// Test ping-pong messages
TEST_F(AuthTest, PingPong) {
    // Create test server
    BattleNetServer server(m_config, m_dbManager);
    
    // Create mock client
    MockBattleNetClient client(server, m_config, m_dbManager);
    
    // Create ping message
    std::vector<uint8_t> pingMessage = {0x03};
    
    // Process the message
    ASSERT_TRUE(client.processTestMessage(pingMessage));
    
    // Check response message
    auto response = client.getLastSentMessage();
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response[0], 0x04); // Pong message type
}

} // namespace test
} // namespace battle_net
} // namespace d3server 