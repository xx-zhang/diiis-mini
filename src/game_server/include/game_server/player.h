#ifndef D3SERVER_PLAYER_H
#define D3SERVER_PLAYER_H

#include <string>
#include <cstdint>
#include <memory>
// #include "game_server/world.h" // Forward declare or include if player has a direct world reference
// #include "database/character_data.h" // If player loads from a struct

namespace d3server {
namespace game_server {

class World; // Forward declaration

class Player : public std::enable_shared_from_this<Player> {
public:
    Player(uint32_t accountId, uint64_t characterId /*, network_session_details */);
    ~Player();

    bool loadCharacterData(std::shared_ptr<d3server::database::DatabaseManager> dbManager);
    bool saveCharacterData(std::shared_ptr<d3server::database::DatabaseManager> dbManager);

    void update(float deltaTime); // For player-specific logic updates

    // Getters
    uint32_t getAccountId() const { return m_accountId; }
    uint64_t getCharacterId() const { return m_characterId; }
    const std::string& getName() const { return m_name; }
    // std::shared_ptr<World> getCurrentWorld() const { return m_currentWorld; }

    // Setters / Actions
    // void setCurrentWorld(std::shared_ptr<World> world);
    // void moveTo(float x, float y, float z);
    // void useItem(uint64_t itemId);
    // ... other player actions

private:
    uint32_t m_accountId;
    uint64_t m_characterId;
    std::string m_name;
    // Player stats, inventory, position, current world, etc.
    // int m_level;
    // int m_health;
    // float m_posX, m_posY, m_posZ;
    // std::weak_ptr<World> m_currentWorld; // Weak to avoid circular refs if World also holds players
    // std::vector<Item> m_inventory;
};

} // namespace game_server
} // namespace d3server

#endif // D3SERVER_PLAYER_H 