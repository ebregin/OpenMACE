#ifndef MAVLINK_CONTROLLER_GPS_GLOBAL_ORIGIN_H
#define MAVLINK_CONTROLLER_GPS_GLOBAL_ORIGIN_H

#include <mavlink.h>

#include "common/common.h"

#include "controllers/actions/action_send.h"
#include "controllers/actions/action_finish.h"

#include "module_vehicle_MAVLINK/mavlink_entity_key.h"

#include "module_vehicle_MAVLINK/controllers/common.h"
#include "module_vehicle_MAVLINK/mavlink_coordinate_frames.h"

namespace MAVLINKUXVControllers {

template <typename T>
using GuidedMISend = Controllers::ActionSend<
    mavlink_message_t,
    MavlinkEntityKey,
    BasicMavlinkController_ModuleKeyed<T>,
    MavlinkEntityKey,
    T,
    mavlink_set_gps_global_origin_t,
    MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN
>;

template <typename T>
using GuidedMIFinish = Controllers::ActionFinish<
    mavlink_message_t,
    MavlinkEntityKey,
    BasicMavlinkController_ModuleKeyed<T>,
    MavlinkEntityKey,
    uint8_t,
    mavlink_gps_global_origin_t,
    MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN
>;

template <typename MISSIONITEM>
class Controller_GPSGlobalOrigin : public BasicMavlinkController_ModuleKeyed<MISSIONITEM>,
        public GuidedMISend<MISSIONITEM>,
        public GuidedMIFinish<MISSIONITEM>
{
private:

    std::unordered_map<MavlinkEntityKey, MavlinkEntityKey> m_CommandRequestedFrom;

protected:

    virtual bool Construct_Send(const MISSIONITEM &commandItem, const MavlinkEntityKey &sender, const MavlinkEntityKey &target, mavlink_mission_item_t &missionItem, MavlinkEntityKey &queueObj)
    {
        UNUSED(sender);
        UNUSED(target);
        queueObj = this->GetKeyFromSecondaryID(commandItem.getTargetSystem());

        missionItem = initializeMAVLINKMissionItem();
        missionItem.target_system = commandItem.getTargetSystem();
        missionItem.target_component = 0;

        FillMissionItem(commandItem,missionItem);

        return true;
    }


    virtual bool Finish_Receive(const mavlink_mission_ack_t &msg, const MavlinkEntityKey &sender, uint8_t & ack, MavlinkEntityKey &queueObj)
    {
        UNUSED(msg);
        queueObj = sender;
        ack = msg.type; //this is MAV_MISSION_RESULT
        return true;
    }

protected:
    void FillMissionItem(const command_item::SpatialWaypoint &commandItem, mavlink_set_gps_global_origin_t &mavlinkItem);

    mavlink_set_gps_global_origin_t initializeGlobalOrigin()
    {
        mavlink_set_gps_global_origin_t item;
        item.target_system = 0;
        item.latitude = 0.0;
        item.longitude = 0.0;
        item.altitude = 0.0;

        return item;
    }

public:
    Controller_GPSGlobalOrigin(const Controllers::IMessageNotifier<mavlink_message_t, MavlinkEntityKey> *cb, TransmitQueue *queue, int linkChan) :
        BasicMavlinkController_ModuleKeyed<MISSIONITEM>(cb, queue, linkChan),
        GuidedMISend<MISSIONITEM>(this, MavlinkEntityKeyToSysIDCompIDConverter<mavlink_mission_item_t>(mavlink_msg_mission_item_encode_chan)),
        GuidedMIFinish<MISSIONITEM>(this, mavlink_msg_mission_ack_decode)
    {

    }

    virtual ~Controller_GPSGlobalOrigin() = default;

};

} //end of namespace MAVLINKUXVControllers

#endif // MAVLINK_CONTROLLER_GPS_GLOBAL_ORIGIN_H
