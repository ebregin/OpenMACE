#ifndef CONTROLLER_GUIDED_TARGET_ITEM_LOCAL_H
#define CONTROLLER_GUIDED_TARGET_ITEM_LOCAL_H

#include <mavlink.h>

#include "common/common.h"

#include "controllers/actions/action_send.h"
#include "controllers/actions/action_finish.h"
#include "controllers/actions/action_broadcast.h"

#include "mavlink.h"

#include "module_vehicle_MAVLINK/mavlink_entity_key.h"

#include "module_vehicle_MAVLINK/controllers/common.h"

#include "data_generic_command_item/do_items/action_dynamic_target.h"

using namespace mace::pose;

namespace MAVLINKVehicleControllers {

template <typename T>
using GuidedTGTLocalBroadcast = Controllers::ActionBroadcast<
    mavlink_message_t,
    MavlinkEntityKey,
    BasicMavlinkController_ModuleKeyed<T>,
    T,
    mavlink_set_position_target_local_ned_t
>;

template <typename COMMANDDATASTRUCTURE>
class ControllerGuidedTargetItem_Local : public BasicMavlinkController_ModuleKeyed<COMMANDDATASTRUCTURE>,
        public GuidedTGTLocalBroadcast<COMMANDDATASTRUCTURE>
{
private:

    std::unordered_map<MavlinkEntityKey, MavlinkEntityKey> m_CommandRequestedFrom;

    mavlink_set_position_target_local_ned_t m_targetMSG;

private:
    bool doesMatchTransmitted(const mavlink_position_target_local_ned_t &msg) const;

protected:


    virtual void Construct_Broadcast(const COMMANDDATASTRUCTURE &commandItem, const MavlinkEntityKey &sender, mavlink_set_position_target_local_ned_t &targetItem)
    {
        UNUSED(sender);

        targetItem = initializeMAVLINKTargetItem();
        targetItem.target_system = commandItem.getTargetSystem();
        targetItem.target_component = static_cast<uint8_t>(MaceCore::ModuleClasses::VEHICLE_COMMS);

        FillTargetItem(commandItem,targetItem);
    }

protected:
    void FillTargetItem(const COMMANDDATASTRUCTURE &targetItem, mavlink_set_position_target_local_ned_t &mavlinkItem);

    mavlink_set_position_target_local_ned_t initializeMAVLINKTargetItem()
    {
        mavlink_set_position_target_local_ned_t targetItem;
        targetItem.afx = 0.0;
        targetItem.afy = 0.0;
        targetItem.afz = 0.0;
        targetItem.coordinate_frame = MAV_FRAME_LOCAL_NED;
        targetItem.target_component =0;
        targetItem.target_system = 0;
        targetItem.time_boot_ms = 0;
        targetItem.type_mask = 65535; //by default we want to ignore all of the values
        targetItem.vx = 0.0;
        targetItem.vy = 0.0;
        targetItem.vz = 0.0;
        targetItem.x = 0.0;
        targetItem.y = 0.0;
        targetItem.yaw_rate = 0.0;
        targetItem.z = 0.0;

        return targetItem;
    }

public:
    ControllerGuidedTargetItem_Local(const Controllers::IMessageNotifier<mavlink_message_t, MavlinkEntityKey> *cb, TransmitQueue *queue, int linkChan) :
        BasicMavlinkController_ModuleKeyed<COMMANDDATASTRUCTURE>(cb, queue, linkChan),
        GuidedTGTLocalBroadcast<COMMANDDATASTRUCTURE>(this, MavlinkEntityKeyToSysIDCompIDConverter<mavlink_set_position_target_local_ned_t>(mavlink_msg_set_position_target_local_ned_encode_chan))
    {

    }

};

} //end of namespace MAVLINKVehicleControllers


#endif // CONTROLLER_GUIDED_TARGET_ITEM_LOCAL_H
