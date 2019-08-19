#ifndef STATE_FLIGHT_GUIDED_TARGET_CAR_H
#define STATE_FLIGHT_GUIDED_TARGET_CAR_H

#include <iostream>

#include "data/timer.h"

#include "abstract_state_ardupilot.h"

#include "../guided_timeout_controller.h"

#include "module_vehicle_MAVLINK/controllers/controller_guided_target_item_local.h"
#include "module_vehicle_MAVLINK/controllers/controller_guided_target_item_global.h"

#include "data_generic_command_item/command_item_components.h"

#include "data_generic_mission_item_topic/mission_item_reached_topic.h"


namespace ardupilot{
namespace state{

class State_FlightGuided_Idle;
class State_FlightGuided_MissionItem;
class State_FlightGuided_Queue;
class State_FlightGuided_GeoTarget;

class State_FlightGuided_CarTarget : public AbstractStateArdupilot
{
public:
    State_FlightGuided_CarTarget();

    void OnExit() override;

public:
    AbstractStateArdupilot* getClone() const override;

    void getClone(AbstractStateArdupilot** state) const override;

public:
    hsm::Transition GetTransition() override;

public:
    bool handleCommand(const std::shared_ptr<AbstractCommandItem> command) override;

    void Update() override;

    void OnEnter() override;

    void OnEnter(const std::shared_ptr<AbstractCommandItem> command) override;

private:
    static void retransmitGuidedCommand(void *p, command_target::DynamicTarget &target)
    {
        static_cast<State_FlightGuided_CarTarget*>(p)->constructAndSendTarget(target);
    }

    void constructAndSendTarget(const command_target::DynamicTarget &command)
    {
        MavlinkEntityKey target = Owner().getMAVLINKID();
        MavlinkEntityKey sender = 255;
        MAVLINKVehicleControllers::TargetControllerStructLocal tgt;
        tgt.targetID = static_cast<uint8_t>(target);
        tgt.target = command;
        static_cast<MAVLINKVehicleControllers::ControllerGuidedTargetItem_Local*>(Owner().ControllersCollection()->At("CartesianTargetController"))->Broadcast(tgt, sender);
    }

private:
    ardupilot_vehicle::GuidedTimeoutController m_TimeoutController;
};

} //end of namespace state
} //end of namespace arudpilot

#endif // STATE_FLIGHT_GUIDED_TARGET_CAR_H