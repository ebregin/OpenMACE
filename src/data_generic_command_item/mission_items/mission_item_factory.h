#ifndef MISSION_ITEM_FACTORY_H
#define MISSION_ITEM_FACTORY_H

#include "../spatial_items/spatial_components.h"
#include "../do_items/do_components.h"
#include "../abstract_command_item.h"

#include "mission_key.h"

namespace MissionItem {

class MissionItemFactory
{
public:
    MissionItemFactory() = default;

    static command_item::AbstractCommandItemPtr generateAbstractCommandItem(const mace_mission_item_t &maceItem, const unsigned int &targetID, const unsigned int originatingID);

    static void generateMACEMissionItem(const command_item::AbstractCommandItemPtr maceItem, const unsigned int &itemIndex, mace_mission_item_t &missionItem);

    static void updateMissionKey(const MissionItem::MissionKey &key, mace_mission_item_t &missionItem);

};

} //end of namespace MissionItem

#endif // MISSION_ITEM_FACTORY_H
