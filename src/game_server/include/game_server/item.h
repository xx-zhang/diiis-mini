#ifndef D3SERVER_ITEM_H
#define D3SERVER_ITEM_H

#include <string>
#include <cstdint>
#include <vector>

namespace d3server {
namespace game_server {

// Enum for item quality
enum class ItemQuality {
    COMMON,
    MAGIC,
    RARE,
    LEGENDARY,
    SET
};

// Enum for item type/slot
enum class ItemType {
    WEAPON_ONE_HAND,
    WEAPON_TWO_HAND,
    OFFHAND_SHIELD,
    OFFHAND_QUIVER,
    OFFHAND_SOURCE,
    HELM,
    CHEST,
    PANTS,
    BOOTS,
    GLOVES,
    SHOULDERS,
    AMULET,
    RING,
    BELT,
    POTION,
    SCROLL,
    GEM,
    MATERIAL
    // ... and so on
};

struct ItemAffix {
    std::string affixName; // e.g., "Strength_Minor"
    float value1;
    float value2;
    // Could also be an ID referencing a static affix definition
};

class Item {
public:
    Item(uint64_t itemId, uint32_t baseItemSno); // item_id is unique instance, baseItemSno is static definition
    ~Item();

    uint64_t getItemId() const { return m_itemId; }
    uint32_t getBaseItemSno() const { return m_baseItemSno; }
    const std::string& getName() const { return m_name; } // Usually from base item definition
    ItemQuality getQuality() const { return m_quality; }
    ItemType getType() const { return m_type; }
    int getRequiredLevel() const { return m_requiredLevel; }
    const std::vector<ItemAffix>& getAffixes() const { return m_affixes; }

    // void generateMagicAffixes(int numAffixes);
    // void generateRareAffixes(int numPrefixes, int numSuffixes);
    // void setToLegendary(uint32_t legendaryItemSno); // Load stats from a legendary definition

private:
    uint64_t m_itemId; // Unique instance ID for this item
    uint32_t m_baseItemSno; // SNO (Serial Number Object - unique ID) for the base item type from game data
    
    std::string m_name; // Display name (often derived from base item + affixes)
    ItemQuality m_quality;
    ItemType m_type;
    int m_itemLevel; // Power level of the item, distinct from required level
    int m_requiredLevel; // Level player needs to be to equip/use
    // Texture/model info, stack size etc.
    
    std::vector<ItemAffix> m_affixes;
    // Other dynamic properties (durability, socketed gems etc.)
};

} // namespace game_server
} // namespace d3server

#endif // D3SERVER_ITEM_H 