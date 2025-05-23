#include <gtest/gtest.h>
#include "database/database_manager.h"
#include "core/config.h"
#include <memory>
#include <string>

namespace d3server {
namespace database {
namespace test {

class AccountDaoTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<DatabaseManager> m_dbManager;
};

// Test account creation
TEST_F(AccountDaoTest, CreateAccount) {
    // Create test account
    EXPECT_TRUE(m_dbManager->createAccount("testuser", "test@example.com", "password123"));
    
    // Verify account exists
    EXPECT_TRUE(m_dbManager->accountExists("testuser"));
    
    // Attempt to create duplicate account (should fail)
    EXPECT_FALSE(m_dbManager->createAccount("testuser", "another@example.com", "password456"));
}

// Test retrieving account
TEST_F(AccountDaoTest, GetAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("getuser", "get@example.com", "password123"));
    
    // Retrieve account
    Account account = m_dbManager->getAccount("getuser");
    
    // Verify account data
    EXPECT_EQ(account.login, "getuser");
    EXPECT_EQ(account.email, "get@example.com");
    EXPECT_FALSE(account.banned);
    EXPECT_EQ(account.characterCount, 0);
}

// Test updating account
TEST_F(AccountDaoTest, UpdateAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("updateuser", "original@example.com", "password123"));
    
    // Update email
    EXPECT_TRUE(m_dbManager->updateAccount("updateuser", "updated@example.com", ""));
    
    // Verify update
    Account account = m_dbManager->getAccount("updateuser");
    EXPECT_EQ(account.email, "updated@example.com");
    
    // Update password
    EXPECT_TRUE(m_dbManager->updateAccount("updateuser", "", "newpassword"));
    
    // Verify password update (by authenticating)
    EXPECT_TRUE(m_dbManager->verifyAccountPassword("updateuser", "newpassword"));
    EXPECT_FALSE(m_dbManager->verifyAccountPassword("updateuser", "password123"));
}

// Test banning account
TEST_F(AccountDaoTest, BanAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("banuser", "ban@example.com", "password123"));
    
    // Ban account
    EXPECT_TRUE(m_dbManager->setBanStatus("banuser", true, "Test ban reason"));
    
    // Verify ban status
    EXPECT_TRUE(m_dbManager->isAccountBanned("banuser"));
    
    // Unban account
    EXPECT_TRUE(m_dbManager->setBanStatus("banuser", false, ""));
    
    // Verify unbanned
    EXPECT_FALSE(m_dbManager->isAccountBanned("banuser"));
}

// Test deleting account
TEST_F(AccountDaoTest, DeleteAccount) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("deleteuser", "delete@example.com", "password123"));
    
    // Verify account exists
    EXPECT_TRUE(m_dbManager->accountExists("deleteuser"));
    
    // Delete account
    EXPECT_TRUE(m_dbManager->deleteAccount("deleteuser"));
    
    // Verify account no longer exists
    EXPECT_FALSE(m_dbManager->accountExists("deleteuser"));
    
    // Attempt to delete non-existent account (should fail)
    EXPECT_FALSE(m_dbManager->deleteAccount("nonexistent"));
}

// Test password verification
TEST_F(AccountDaoTest, PasswordVerification) {
    // Create test account
    ASSERT_TRUE(m_dbManager->createAccount("passuser", "pass@example.com", "correctpassword"));
    
    // Test password verification
    EXPECT_TRUE(m_dbManager->verifyAccountPassword("passuser", "correctpassword"));
    EXPECT_FALSE(m_dbManager->verifyAccountPassword("passuser", "wrongpassword"));
    EXPECT_FALSE(m_dbManager->verifyAccountPassword("nonexistent", "password"));
}

// Test getting all accounts
TEST_F(AccountDaoTest, GetAllAccounts) {
    // Create multiple test accounts
    ASSERT_TRUE(m_dbManager->createAccount("user1", "user1@example.com", "password1"));
    ASSERT_TRUE(m_dbManager->createAccount("user2", "user2@example.com", "password2"));
    ASSERT_TRUE(m_dbManager->createAccount("user3", "user3@example.com", "password3"));
    
    // Get all accounts
    std::vector<Account> accounts = m_dbManager->getAllAccounts();
    
    // Verify accounts count
    EXPECT_GE(accounts.size(), 3);
    
    // Verify account data (this is a simplistic check)
    bool foundUser1 = false;
    bool foundUser2 = false;
    bool foundUser3 = false;
    
    for (const auto& account : accounts) {
        if (account.login == "user1") foundUser1 = true;
        if (account.login == "user2") foundUser2 = true;
        if (account.login == "user3") foundUser3 = true;
    }
    
    EXPECT_TRUE(foundUser1);
    EXPECT_TRUE(foundUser2);
    EXPECT_TRUE(foundUser3);
}

// Test account count
TEST_F(AccountDaoTest, AccountCount) {
    // Get initial count
    int initialCount = m_dbManager->getAccountCount();
    
    // Create multiple test accounts
    ASSERT_TRUE(m_dbManager->createAccount("countuser1", "count1@example.com", "password1"));
    ASSERT_TRUE(m_dbManager->createAccount("countuser2", "count2@example.com", "password2"));
    
    // Verify count increased
    EXPECT_EQ(m_dbManager->getAccountCount(), initialCount + 2);
    
    // Delete an account
    ASSERT_TRUE(m_dbManager->deleteAccount("countuser1"));
    
    // Verify count decreased
    EXPECT_EQ(m_dbManager->getAccountCount(), initialCount + 1);
}

} // namespace test
} // namespace database
} // namespace d3server 