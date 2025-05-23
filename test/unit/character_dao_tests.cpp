#include <gtest/gtest.h>
#include "database/database_manager.h"
#include "core/config.h"
#include <memory>
#include <string>

namespace d3server {
namespace database {
namespace test {

class CharacterDaoTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test dependencies
        m_config = std::make_shared<core::Config>();
        ASSERT_TRUE(m_config->init(":memory:"));
        
        m_dbManager = std::make_shared<DatabaseManager>(m_config);
        ASSERT_TRUE(m_dbManager->init());
        
        // Create test account
        ASSERT_TRUE(m_dbManager->createAccount("testaccount", "test@example.com", "password123"));
    }
    
    void TearDown() override {
        m_dbManager.reset();
        m_config.reset();
    }
    
    // Helper to create a character
    Character createTestCharacter(const std::string& name, int classId, int level = 1) {
        Character character;
        character.name = name;
        character.classId = classId;
        character.level = level;
        return character;
    }
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<DatabaseManager> m_dbManager;
};

// Test character creation
TEST_F(CharacterDaoTest, CreateCharacter) {
    // Create character
    Character character = createTestCharacter("Barbarian1", 1, 10);
    
    // Add character to account
    EXPECT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Verify character exists
    EXPECT_TRUE(m_dbManager->characterExists("Barbarian1"));
    
    // Attempt to create duplicate character (should fail)
    EXPECT_FALSE(m_dbManager->createCharacter("testaccount", character));
}

// Test getting all characters for an account
TEST_F(CharacterDaoTest, GetAllCharacters) {
    // Create multiple characters
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Barbarian1", 1, 10)));
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Wizard1", 3, 15)));
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Monk1", 2, 5)));
    
    // Get characters
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    
    // Verify character count
    EXPECT_EQ(characters.size(), 3);
    
    // Verify character data (this is a simplistic check)
    bool foundBarbarian = false;
    bool foundWizard = false;
    bool foundMonk = false;
    
    for (const auto& character : characters) {
        if (character.name == "Barbarian1") {
            foundBarbarian = true;
            EXPECT_EQ(character.classId, 1);
            EXPECT_EQ(character.level, 10);
        }
        else if (character.name == "Wizard1") {
            foundWizard = true;
            EXPECT_EQ(character.classId, 3);
            EXPECT_EQ(character.level, 15);
        }
        else if (character.name == "Monk1") {
            foundMonk = true;
            EXPECT_EQ(character.classId, 2);
            EXPECT_EQ(character.level, 5);
        }
    }
    
    EXPECT_TRUE(foundBarbarian);
    EXPECT_TRUE(foundWizard);
    EXPECT_TRUE(foundMonk);
}

// Test getting character count
TEST_F(CharacterDaoTest, GetCharacterCount) {
    // Verify initial count
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), 0);
    
    // Create multiple characters
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Char1", 1)));
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", createTestCharacter("Char2", 2)));
    
    // Verify count
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), 2);
}

// Test max character limit
TEST_F(CharacterDaoTest, MaxCharacterLimit) {
    // Get max characters per account from config
    int maxCharacters = m_config->getServerConfig().maxCharactersPerAccount;
    
    // If max characters is not set or unreasonably high, use a smaller value for testing
    if (maxCharacters <= 0 || maxCharacters > 20) {
        maxCharacters = 5;
        
        // Set max characters in config
        auto serverConfig = m_config->getServerConfig();
        serverConfig.maxCharactersPerAccount = maxCharacters;
        m_config->updateServerConfig(serverConfig);
    }
    
    // Create characters up to the limit
    for (int i = 0; i < maxCharacters; i++) {
        ASSERT_TRUE(m_dbManager->createCharacter(
            "testaccount", 
            createTestCharacter("Character" + std::to_string(i + 1), (i % 7) + 1)
        ));
    }
    
    // Verify count
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), maxCharacters);
    
    // Attempt to create one more character (should fail)
    EXPECT_FALSE(m_dbManager->createCharacter(
        "testaccount", 
        createTestCharacter("ExtraCharacter", 1)
    ));
    
    // Verify count hasn't changed
    EXPECT_EQ(m_dbManager->getCharacterCount("testaccount"), maxCharacters);
}

// Test retrieving character by ID
TEST_F(CharacterDaoTest, GetCharacterById) {
    // Create character
    Character character = createTestCharacter("GetByIdTest", 3, 25);
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Get all characters to find the ID
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    ASSERT_FALSE(characters.empty());
    
    // Find the character we just created
    uint32_t characterId = 0;
    for (const auto& c : characters) {
        if (c.name == "GetByIdTest") {
            characterId = c.id;
            break;
        }
    }
    
    // Verify character ID is valid
    ASSERT_GT(characterId, 0);
    
    // Get character by ID
    Character retrievedCharacter = m_dbManager->getCharacter(characterId);
    
    // Verify character data
    EXPECT_EQ(retrievedCharacter.id, characterId);
    EXPECT_EQ(retrievedCharacter.name, "GetByIdTest");
    EXPECT_EQ(retrievedCharacter.classId, 3);
    EXPECT_EQ(retrievedCharacter.level, 25);
}

// Test updating character
TEST_F(CharacterDaoTest, UpdateCharacter) {
    // Create character
    Character character = createTestCharacter("UpdateTest", 1, 10);
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Get character ID
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    uint32_t characterId = 0;
    for (const auto& c : characters) {
        if (c.name == "UpdateTest") {
            characterId = c.id;
            break;
        }
    }
    ASSERT_GT(characterId, 0);
    
    // Update character
    Character updatedCharacter = m_dbManager->getCharacter(characterId);
    updatedCharacter.level = 20;
    EXPECT_TRUE(m_dbManager->updateCharacter(updatedCharacter));
    
    // Verify update
    Character retrievedCharacter = m_dbManager->getCharacter(characterId);
    EXPECT_EQ(retrievedCharacter.level, 20);
}

// Test deleting character
TEST_F(CharacterDaoTest, DeleteCharacter) {
    // Create character
    Character character = createTestCharacter("DeleteTest", 2, 15);
    ASSERT_TRUE(m_dbManager->createCharacter("testaccount", character));
    
    // Verify character exists
    EXPECT_TRUE(m_dbManager->characterExists("DeleteTest"));
    
    // Get character ID
    std::vector<Character> characters = m_dbManager->getCharacters("testaccount");
    uint32_t characterId = 0;
    for (const auto& c : characters) {
        if (c.name == "DeleteTest") {
            characterId = c.id;
            break;
        }
    }
    ASSERT_GT(characterId, 0);
    
    // Delete character
    EXPECT_TRUE(m_dbManager->deleteCharacter(characterId));
    
    // Verify character no longer exists
    EXPECT_FALSE(m_dbManager->characterExists("DeleteTest"));
    
    // Verify character count decreased
    std::vector<Character> remainingCharacters = m_dbManager->getCharacters("testaccount");
    bool characterFound = false;
    for (const auto& c : remainingCharacters) {
        if (c.name == "DeleteTest") {
            characterFound = true;
            break;
        }
    }
    EXPECT_FALSE(characterFound);
}

} // namespace test
} // namespace database
} // namespace d3server 