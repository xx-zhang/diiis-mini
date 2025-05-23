#ifndef D3SERVER_WORLD_H
#define D3SERVER_WORLD_H

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <mutex>
// #include "game_server/player.h"
// #include "game_server/monster.h"
// #include "game_server/item_drop.h"

namespace d3server {
namespace game_server {

class Player; // Forward declaration
// class Monster; // Forward declaration
// class ItemDrop; // Forward declaration

class World : public std::enable_shared_from_this<World> {
public:
    World(const std::string& name, const std::string& scene_identifier /*, GameServer* gameServer */);
    ~World();

    void init(); // Load scene data, spawn initial monsters etc.
    void update(float deltaTime); // Update all entities in the world

    void addPlayer(std::shared_ptr<Player> player);
    void removePlayer(std::shared_ptr<Player> player);

    // void spawnMonster(uint32_t monster_id, float x, float y, float z);
    // void spawnItemDrop(uint32_t item_id, float x, float y, float z, std::shared_ptr<Player> owner = nullptr);

    const std::string& getName() const { return m_name; }

private:
    std::string m_name;
    std::string m_sceneIdentifier; // e.g., "tristram_cathedral_level1"
    // GameServer* m_gameServerRef; // Non-owning raw pointer to owning GameServer for callbacks

    std::set<std::shared_ptr<Player>> m_playersInWorld;
    std::mutex m_playersMutex;

    // std::vector<std::shared_ptr<Monster>> m_monsters;
    // std::mutex m_monstersMutex;

    // std::vector<std::shared_ptr<ItemDrop>> m_itemDrops;
    // std::mutex m_itemDropsMutex;

    // Scene geometry, collision data, etc.
};

} // namespace game_server
} // namespace d3server

#endif // D3SERVER_WORLD_H 