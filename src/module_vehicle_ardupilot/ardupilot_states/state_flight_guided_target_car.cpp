#include "state_flight_guided_target_car.h"

namespace ardupilot{
namespace state{

State_FlightGuided_CarTarget::State_FlightGuided_CarTarget():
    AbstractStateArdupilot(), m_TimeoutController(500)
{
    std::cout<<"We are in the constructor of STATE_FLIGHT_GUIDED_CARTARGET"<<std::endl;
    currentStateEnum = ArdupilotFlightState::STATE_FLIGHT_GUIDED_CARTARGET;
    desiredStateEnum = ArdupilotFlightState::STATE_FLIGHT_GUIDED_CARTARGET;

    m_TimeoutController.connectTargetCallback(State_FlightGuided_CarTarget::retransmitGuidedCommand, this);
}

void State_FlightGuided_CarTarget::OnExit()
{
    AbstractStateArdupilot::OnExit();
    Owner().state->vehicleGlobalPosition.RemoveNotifier(this);
    if(Owner().ControllersCollection()->Exist("CartesianTargetController")){
        MAVLINKVehicleControllers::ControllerGuidedTargetItem_Local* ptr = dynamic_cast<MAVLINKVehicleControllers::ControllerGuidedTargetItem_Local*>(Owner().ControllersCollection()->Remove("CartesianTargetController"));
        delete ptr;
    }

}

AbstractStateArdupilot* State_FlightGuided_CarTarget::getClone() const
{
    return (new State_FlightGuided_CarTarget(*this));
}

void State_FlightGuided_CarTarget::getClone(AbstractStateArdupilot** state) const
{
    *state = new State_FlightGuided_CarTarget(*this);
}

hsm::Transition State_FlightGuided_CarTarget::GetTransition()
{
    hsm::Transition rtn = hsm::NoTransition();

    if(currentStateEnum != desiredStateEnum)
    {
        //this means we want to chage the state of the vehicle for some reason
        //this could be caused by a command, action sensed by the vehicle, or
        //for various other peripheral reasons
        switch (desiredStateEnum) {
        case ArdupilotFlightState::STATE_FLIGHT_GUIDED_IDLE:
        {
            rtn = hsm::SiblingTransition<State_FlightGuided_Idle>(currentCommand);
            break;
        }
        case ArdupilotFlightState::STATE_FLIGHT_GUIDED_MISSIONITEM:
        {
            rtn = hsm::SiblingTransition<State_FlightGuided_Idle>(currentCommand);
            break;
        }
        default:
            std::cout<<"I dont know how we eneded up in this transition state from State_FlightGuided_Target."<<std::endl;
            break;
        }
    }
    return rtn;
}

bool State_FlightGuided_CarTarget::handleCommand(const std::shared_ptr<AbstractCommandItem> command)
{
    bool commandHandled = false;

    switch (command->getCommandType()) {
    case COMMANDTYPE::CI_ACT_TARGET:
    {
        //stop the current controllers target transmission if it is running
        m_TimeoutController.clearTarget();

        //We want to keep this command in scope to perform the action
        this->currentCommand = command->getClone();

        //The command is a target, we therefore have to figure out what type of target it is
        const command_item::Action_DynamicTarget* cmd = currentCommand->as<command_item::Action_DynamicTarget>();

        const mace::pose::Position* targetPosition = cmd->getDynamicTarget().getPosition();
        if(targetPosition != nullptr)
        {
            /* Since the positional element within the command is not null, we have to make sure it
             * is of the coordinate frame type that can be supported within this guided mode.
             * If it is not, we will update the desired state for th state machine and switch.
             */
            if(targetPosition->getCoordinateSystemType() == CoordinateSystemTypes::GEODETIC)
            {
                desiredStateEnum = ArdupilotFlightState::STATE_FLIGHT_GUIDED_GEOTARGET;
                commandHandled = false;
                break;
            }
        }

        constructAndSendTarget(cmd->getDynamicTarget());

        /*
         * Determine if the velocity component is valid, and if so, update the timeout controller
         * with the appropriate target to ensure that upon the designated timeout, the controller
         * retransmits the command to the ardupilot.
         *
         * NOTE: Ardupilot requires that all the velocities be valid
         */
        if(cmd->getDynamicTarget().getVelocity()->areAllVelocitiesValid())
        {
            m_TimeoutController.registerCurrentTarget(cmd->getDynamicTarget());
        }
        commandHandled = true;
        break;
    }
    case COMMANDTYPE::CI_ACT_MISSIONITEM:
    {
        this->currentCommand = command->getClone();
        desiredStateEnum = ArdupilotFlightState::STATE_FLIGHT_GUIDED_MISSIONITEM;
    }
    default:
        break;
    }

    return commandHandled;
}

void State_FlightGuided_CarTarget::Update()
{

}

void State_FlightGuided_CarTarget::OnEnter()
{
    /*
     * For some reason we have gotten into this state without a command,
     * or the command is null. We therefore will return to the idle state
     * of the guided flight mode.
     */
    desiredStateEnum = ArdupilotFlightState::STATE_FLIGHT_GUIDED_IDLE;
}

void State_FlightGuided_CarTarget::OnEnter(const std::shared_ptr<AbstractCommandItem> command)
{
    if(command == nullptr)
    {
        this->OnEnter();
        return;
    }

    if(command->getCommandType() != COMMANDTYPE::CI_ACT_TARGET)
    {
        //we dont handle commands of this type in here
        desiredStateEnum = ArdupilotFlightState::STATE_FLIGHT_GUIDED_IDLE;
        return;
    }


    //Insert a new controller only one time in the guided state to manage the entirity of the commands that are of the dynamic target type
    Controllers::ControllerCollection<mavlink_message_t, MavlinkEntityKey> *collection = Owner().ControllersCollection();

    auto cartesianTargetController = new MAVLINKVehicleControllers::ControllerGuidedTargetItem_Local(&Owner(), Owner().GetControllerQueue(), Owner().getCommsObject()->getLinkChannel());

    collection->Insert("CartesianTargetController",cartesianTargetController);

    this->handleCommand(command);
}

} //end of namespace ardupilot
} //end of namespace state

#include "ardupilot_states/state_flight_guided_idle.h"
#include "ardupilot_states/state_flight_guided_mission_item.h"
#include "ardupilot_states/state_flight_guided_queue.h"
#include "ardupilot_states/state_flight_guided_target_geo.h"
