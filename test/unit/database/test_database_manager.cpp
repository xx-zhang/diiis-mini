#include "gtest/gtest.h"
#include "database/database_manager.h"
#include "utils/crypto_utils.h" // For potential direct use or if db_manager uses it visibly
#include "core/logger.h" // For initializing logger if tests require it
#include <filesystem> // For managing test database file

// Test fixture for DatabaseManager tests
class DatabaseManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<d3server::database::DatabaseManager> dbManager;
    const std::string testDbPath = "test_d3server.sqlite";

    void SetUp() override {
        // Initialize logger (optional, but good practice if dbManager logs)
        d3server::core::Logger::getInstance()->init("test_db_log.txt", true, d3server::core::LogLevel::DEBUG);

        // Remove any existing test database file
        std::filesystem::remove(testDbPath);

        dbManager = std::make_shared<d3server::database::DatabaseManager>();
        ASSERT_TRUE(dbManager->init(testDbPath));
        ASSERT_TRUE(dbManager->createTables());
    }

    void TearDown() override {
        // Clean up the test database file
        dbManager.reset(); // Release DatabaseManager which should close the db
        std::filesystem::remove(testDbPath); // Explicitly remove after closing
    }
};

TEST_F(DatabaseManagerTest, AccountCreationAndVerification) {
    std::string login = "testuser";
    std::string email = "test@example.com";
    std::string password = "password123";

    // 1. Create account
    ASSERT_TRUE(dbManager->createAccount(login, email, password)) << "Failed to create account.";

    // 2. Check if account exists
    ASSERT_TRUE(dbManager->accountExists(login)) << "Account should exist after creation.";
    ASSERT_FALSE(dbManager->accountExists("nonexistentuser")) << "Non-existent account reported as existing.";

    // 3. Verify password
    ASSERT_TRUE(dbManager->verifyAccountPassword(login, password)) << "Password verification failed for correct password.";
    ASSERT_FALSE(dbManager->verifyAccountPassword(login, "wrongpassword")) << "Password verification succeeded for incorrect password.";
    ASSERT_FALSE(dbManager->verifyAccountPassword("nonexistentuser", password)) << "Password verification succeeded for non-existent user.";
    
    // 4. Check ban status (should be false by default)
    ASSERT_FALSE(dbManager->isAccountBanned(login)) << "Newly created account should not be banned.";

    // 5. Update last login
    ASSERT_TRUE(dbManager->updateAccountLastLogin(login)) << "Failed to update last login time.";
    // We can't easily verify the exact time here without more complex DB queries/mocks,
    // but we check if the call succeeds.
}

TEST_F(DatabaseManagerTest, DuplicateAccountCreation) {
    std::string login = "testuser";
    std::string email = "test@example.com";
    std::string password = "password123";

    ASSERT_TRUE(dbManager->createAccount(login, email, password));
    ASSERT_FALSE(dbManager->createAccount(login, "another@example.com", "anotherpass")) << "Should not be able to create account with duplicate login.";
}

TEST_F(DatabaseManagerTest, NonExistentAccountOperations) {
    ASSERT_FALSE(dbManager->accountExists("nosuchuser"));
    ASSERT_FALSE(dbManager->verifyAccountPassword("nosuchuser", "anypassword"));
    ASSERT_FALSE(dbManager->isAccountBanned("nosuchuser"));
    ASSERT_FALSE(dbManager->updateAccountLastLogin("nosuchuser"));
}

// Add more tests for other DatabaseManager functionalities as they are implemented
// e.g., character management, banning/unbanning accounts, etc.

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 