#include "game_server/world.h"
#include "d3core/logger.h"
#include "utils/debug.h"
#include "game_server/player.h" // Include for std::shared_ptr<Player>

namespace d3server {
namespace game_server {

World::World(const std::string& name, const std::string& scene_identifier)
    : m_name(name), m_sceneIdentifier(scene_identifier) {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("World created: " + m_name + " (Scene: " + m_sceneIdentifier + ")");
    DEBUG_FUNCTION_EXIT();
}

World::~World() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("World destroyed: " + m_name);
    // TODO: Clean up any entities, notify players etc.
    m_playersInWorld.clear();
    DEBUG_FUNCTION_EXIT();
}

void World::init() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Initializing world: " + m_name);
    // TODO: Load scene data, spawn initial set of monsters, etc.
    DEBUG_FUNCTION_EXIT();
}

void World::update(float deltaTime) {
    // Update all entities in the world (players, monsters, projectiles, etc.)
    std::lock_guard<std::mutex> lock_players(m_playersMutex);
    for (const auto& player : m_playersInWorld) {
        if (player) {
            // player->update(deltaTime); // If player has its own detailed update
        }
    }
    // TODO: Update monsters, items, world events, etc.
}

void World::addPlayer(std::shared_ptr<Player> player) {
    DEBUG_FUNCTION_ENTER();
    if (player) {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        m_playersInWorld.insert(player);
        LOG_INFO("Player " + player->getName() + " (AccID: " + std::to_string(player->getAccountId()) + ") added to world " + m_name);
        // TODO: Notify other players in the vicinity, send world state to new player
    } else {
        LOG_WARNING("Attempted to add a null player to world " + m_name);
    }
    DEBUG_FUNCTION_EXIT();
}

void World::removePlayer(std::shared_ptr<Player> player) {
    DEBUG_FUNCTION_ENTER();
    if (player) {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        auto it = m_playersInWorld.find(player);
        if (it != m_playersInWorld.end()) {
            m_playersInWorld.erase(it);
            LOG_INFO("Player " + player->getName() + " (AccID: " + std::to_string(player->getAccountId()) + ") removed from world " + m_name);
            // TODO: Notify other players
        } else {
            LOG_WARNING("Attempted to remove player " + player->getName() + " not found in world " + m_name);
        }
    } else {
        LOG_WARNING("Attempted to remove a null player from world " + m_name);
    }
    DEBUG_FUNCTION_EXIT();
}

} // namespace game_server
} // namespace d3server 