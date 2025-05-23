#include <gtest/gtest.h>
#include "rest_api/account_controller.h"
#include "core/config.h"
#include "database/database_manager.h"
#include <memory>
#include <string>

namespace d3server {
namespace rest_api {
namespace test {

class AccountControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<database::DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Create account controller
        m_accountController = std::make_shared<AccountController>(m_config, m_dbManager);
    }
    
    void TearDown() override {
        m_accountController.reset();
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Helper to create a test account
    bool createTestAccount(const std::string& login, const std::string& email, const std::string& password) {
        HttpRequest request;
        request.method = "POST";
        request.uri = "/api/accounts";
        request.headers["Content-Type"] = "application/json";
        request.body = "{\"login\":\"" + login + "\",\"email\":\"" + email + "\",\"password\":\"" + password + "\"}";
        
        HttpResponse response = m_accountController->createAccount(request);
        return response.statusCode == 201;
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<AccountController> m_accountController;
};

// Test account creation
TEST_F(AccountControllerTest, CreateAccount) {
    // Create valid request
    HttpRequest request;
    request.method = "POST";
    request.uri = "/api/accounts";
    request.headers["Content-Type"] = "application/json";
    request.body = R"({"login":"testuser","email":"test@example.com","password":"password123"})";
    
    // Call controller
    HttpResponse response = m_accountController->createAccount(request);
    
    // Verify success
    EXPECT_EQ(response.statusCode, 201);
    EXPECT_EQ(response.statusMessage, "Created");
    EXPECT_EQ(response.headers["Content-Type"], "application/json");
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Try creating duplicate account
    response = m_accountController->createAccount(request);
    
    // Verify conflict
    EXPECT_EQ(response.statusCode, 409);
    EXPECT_NE(response.body.find("already exists"), std::string::npos);
}

// Test invalid account creation
TEST_F(AccountControllerTest, CreateAccountInvalid) {
    // Test missing login
    HttpRequest request;
    request.method = "POST";
    request.uri = "/api/accounts";
    request.headers["Content-Type"] = "application/json";
    request.body = R"({"email":"test@example.com","password":"password123"})";
    
    HttpResponse response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
    
    // Test invalid email
    request.body = R"({"login":"testuser2","email":"invalid-email","password":"password123"})";
    response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
    
    // Test short password
    request.body = R"({"login":"testuser3","email":"test@example.com","password":"short"})";
    response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
    
    // Test invalid JSON
    request.body = R"({"login":"testuser4","email":"test@example.com","password":})";
    response = m_accountController->createAccount(request);
    EXPECT_EQ(response.statusCode, 400);
}

// Test getting account
TEST_F(AccountControllerTest, GetAccount) {
    // Create test account
    ASSERT_TRUE(createTestAccount("getuser", "get@example.com", "password123"));
    
    // Create request to get account
    HttpRequest request;
    request.method = "GET";
    request.uri = "/api/accounts/getuser";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    
    // Call controller
    HttpResponse response = m_accountController->getAccount(request, "getuser");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_EQ(response.statusMessage, "OK");
    EXPECT_EQ(response.headers["Content-Type"], "application/json");
    EXPECT_NE(response.body.find("getuser"), std::string::npos);
    EXPECT_NE(response.body.find("get@example.com"), std::string::npos);
    
    // Test non-existent account
    response = m_accountController->getAccount(request, "nonexistent");
    EXPECT_EQ(response.statusCode, 404);
}

// Test updating account
TEST_F(AccountControllerTest, UpdateAccount) {
    // Create test account
    ASSERT_TRUE(createTestAccount("updateuser", "update@example.com", "password123"));
    
    // Create request to update account
    HttpRequest request;
    request.method = "PUT";
    request.uri = "/api/accounts/updateuser";
    request.headers["Content-Type"] = "application/json";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    request.body = R"({"email":"updated@example.com","password":"newpassword123"})";
    
    // Call controller
    HttpResponse response = m_accountController->updateAccount(request, "updateuser");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Get updated account
    request.method = "GET";
    request.uri = "/api/accounts/updateuser";
    response = m_accountController->getAccount(request, "updateuser");
    
    // Verify updated email
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("updated@example.com"), std::string::npos);
}

// Test deleting account
TEST_F(AccountControllerTest, DeleteAccount) {
    // Create test account
    ASSERT_TRUE(createTestAccount("deleteuser", "delete@example.com", "password123"));
    
    // Create request to delete account
    HttpRequest request;
    request.method = "DELETE";
    request.uri = "/api/accounts/deleteuser";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    
    // Call controller
    HttpResponse response = m_accountController->deleteAccount(request, "deleteuser");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Try to get deleted account
    request.method = "GET";
    request.uri = "/api/accounts/deleteuser";
    response = m_accountController->getAccount(request, "deleteuser");
    
    // Verify account not found
    EXPECT_EQ(response.statusCode, 404);
}

// Test getting characters
TEST_F(AccountControllerTest, GetCharacters) {
    // Create test account
    ASSERT_TRUE(createTestAccount("charuser", "char@example.com", "password123"));
    
    // Create request to get characters
    HttpRequest request;
    request.method = "GET";
    request.uri = "/api/accounts/charuser/characters";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    
    // Call controller
    HttpResponse response = m_accountController->getCharacters(request, "charuser");
    
    // Verify success (empty list)
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_EQ(response.headers["Content-Type"], "application/json");
    EXPECT_NE(response.body.find("[]"), std::string::npos);
}

// Test creating character
TEST_F(AccountControllerTest, CreateCharacter) {
    // Create test account
    ASSERT_TRUE(createTestAccount("charuser2", "char2@example.com", "password123"));
    
    // Create request to create character
    HttpRequest request;
    request.method = "POST";
    request.uri = "/api/accounts/charuser2/characters";
    request.headers["Content-Type"] = "application/json";
    request.headers["Authorization"] = "Bearer test_api_key_123456789012345678901234567890123456789";
    request.body = R"({"name":"TestChar","class_id":1})";
    
    // Call controller
    HttpResponse response = m_accountController->createCharacter(request, "charuser2");
    
    // Verify success
    EXPECT_EQ(response.statusCode, 201);
    EXPECT_NE(response.body.find("success"), std::string::npos);
    
    // Get characters
    request.method = "GET";
    request.uri = "/api/accounts/charuser2/characters";
    response = m_accountController->getCharacters(request, "charuser2");
    
    // Verify character was created
    EXPECT_EQ(response.statusCode, 200);
    EXPECT_NE(response.body.find("TestChar"), std::string::npos);
    EXPECT_NE(response.body.find("\"class_id\":1"), std::string::npos);
}

} // namespace test
} // namespace rest_api
} // namespace d3server 