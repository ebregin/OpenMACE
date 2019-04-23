#include "module_external_link_generic.h"

#include <set>

#include "module_generic_MAVLINK/controllers/controller_mavlink_generic_set.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
///             CONFIGURE
////////////////////////////////////////////////////////////////////////////////////////////////////////

void FinishedMessage(const bool completed, const uint8_t finishCode)
{
    if(completed == false)
    {
        printf("Controller timed out sending message, gave up sending message\n");
    }
    else {
        printf("Controller Received Final ACK with code of %d\n", finishCode);
    }
}

template <typename T>
T* Helper_CreateAndSetUp(ModuleExternalLinkGeneric* obj, Controllers::MessageModuleTransmissionQueue<mace_message_t> *queue, uint8_t chan)
{
    T* newController = new T(obj, queue, chan);
    newController->setLambda_DataReceived([obj](const MaceCore::ModuleCharacteristic &moduleFor, const std::shared_ptr<AbstractCommandItem> &command){obj->ReceivedCommand(moduleFor, command);});
    newController->setLambda_Finished(FinishedMessage);
    return newController;
}


ModuleExternalLinkGeneric::ModuleExternalLinkGeneric() :
    associatedSystemID(254), airborneInstance(true),
    m_HeartbeatController(NULL)
{
    m_queue = new Controllers::MessageModuleTransmissionQueue<mace_message_t>(4000, 3);
}

void ModuleExternalLinkGeneric::ReceivedCommand(const MaceCore::ModuleCharacteristic &moduleFor, const std::shared_ptr<AbstractCommandItem> &command)
{
    int mavlinkID = 0;
    if(moduleFor.MaceInstance != 0)
    {
        mavlinkID = this->getDataObject()->getMavlinkIDFromModule(moduleFor);
    }

    command->setTargetSystem(mavlinkID);

    ModuleExternalLinkGeneric::NotifyListeners([&](MaceCore::IModuleEventsGeneral* ptr){
        ptr->Event_IssueGeneralCommand(this, command);
    });
}

ModuleExternalLinkGeneric::~ModuleExternalLinkGeneric()
{

}

std::vector<MaceCore::TopicCharacteristic> ModuleExternalLinkGeneric::GetEmittedTopics()
{
    std::vector<MaceCore::TopicCharacteristic> topics;

    for(auto it = m_SubModules.cbegin() ; it != m_SubModules.cend() ; ++it)
    {
        std::vector<MaceCore::TopicCharacteristic> topicsToAdd = (*it)->GetEmittedTopics();
        topics.insert(topics.cend(), topicsToAdd.cbegin(), topicsToAdd.cend());
    }

    return topics;
}

//!
//! \brief This module as been attached as a module
//! \param ptr pointer to object that attached this instance to itself
//!
void ModuleExternalLinkGeneric::AttachedAsModule(MaceCore::IModuleTopicEvents* ptr)
{
    for(auto it = m_SubModules.cbegin() ; it != m_SubModules.cend() ; ++it)
    {
        (*it)->AttachedAsModule(ptr);
    }
}

//!
//! \brief Describes the strucure of the parameters for this module
//! \return Strucure
//!
std::shared_ptr<MaceCore::ModuleParameterStructure> ModuleExternalLinkGeneric::ModuleConfigurationStructure() const
{
    MaceCore::ModuleParameterStructure structure;
    ConfigureMACEStructure(structure);
    return std::make_shared<MaceCore::ModuleParameterStructure>(structure);

    std::shared_ptr<MaceCore::ModuleParameterStructure> moduleSettings = std::make_shared<MaceCore::ModuleParameterStructure>();
    moduleSettings->AddTerminalParameters("AirborneInstance", MaceCore::ModuleParameterTerminalTypes::BOOLEAN, true);
    structure.AddNonTerminal("ModuleParameters", moduleSettings, true);

    return std::make_shared<MaceCore::ModuleParameterStructure>(structure);

}



//!
//! \brief Provides object contains parameters values to configure module with
//! \param params Parameters to configure
//!
void ModuleExternalLinkGeneric::ConfigureModule(const std::shared_ptr<MaceCore::ModuleParameterValue> &params)
{
    ConfigureMACEComms(params);

    m_LinkMarshaler->SpecifyAddedModuleAction(m_LinkName, [this](const CommsMACE::Resource &resource){
        this->ExternalModuleAdded(resource);
    });

    m_LinkMarshaler->SpecifyAddedModuleAction(m_LinkName, [this](const CommsMACE::Resource &resource){
        this->ExternalModuleRemoved(resource);
    });

    if(params->HasNonTerminal("ModuleParameters"))
    {
        std::shared_ptr<MaceCore::ModuleParameterValue> moduleSettings = params->GetNonTerminalValue("ModuleParameters");
        airborneInstance = moduleSettings->GetTerminalValue<bool>("AirborneInstance");

        //m_LinkMarshaler->AddResource(m_LinkName, GROUNDSTATION_STR, associatedSystemID);

        if(airborneInstance == false)
        {

        }
    }

}

//!
//! \brief Event to fire when an external module has been added
//! \param resourceName Name of resource (module) added
//! \param ID ID of module
//!
void ModuleExternalLinkGeneric::ExternalModuleAdded(const CommsMACE::Resource &resource)
{
    if(strcmp(resource.NameAt(0).c_str(), CommsMACE::MACE_INSTANCE_STR) != 0)
    {
        throw std::runtime_error("The first component of given resource is not the mace instance");
    }

    //If only one component then this is a notification of a MACE instance, not a module.
    if(resource.Size() == 1)
    {
        NewExternalMaceInstance(resource.IDAt(0));
        return;
    }

    MaceCore::ModuleCharacteristic module;
    module.MaceInstance = resource.IDAt(0);
    module.ModuleID = resource.IDAt(1);

    MaceCore::ModuleClasses type;
    const char* moduleTypeName = resource.NameAt(1).c_str();
    if(strcmp(moduleTypeName, CommsMACE::VEHICLE_STR) == 0)
    {
        type = MaceCore::ModuleClasses::VEHICLE_COMMS;
    }
    else if(strcmp(moduleTypeName, CommsMACE::GROUNDSTATION_STR) == 0)
    {
        type = MaceCore::ModuleClasses::GROUND_STATION;
    }
    else if(strcmp(moduleTypeName, CommsMACE::RTA_STR) == 0)
    {
        type = MaceCore::ModuleClasses::RTA;
    }
    else if(strcmp(moduleTypeName, CommsMACE::EXTERNAL_LINK_STR) == 0)
    {
        type = MaceCore::ModuleClasses::EXTERNAL_LINK;
    }
    else {
//        printf("%s\n", CommsMACE::GROUNDSTATION_STR);
        printf("In ExternalModuleAdded - Unknown module class: %s\n", moduleTypeName);
        throw std::runtime_error("Unknown module class received");
    }

    ModuleExternalLinkGeneric::NotifyListeners([&](MaceCore::IModuleEventsExternalLink* ptr){
        ptr->Event_NewModule(this, module, type);
    });

    //check if there are tasks and perform them
    if(m_TasksToDoWhenModuleComesOnline.find(module) != m_TasksToDoWhenModuleComesOnline.cend())
    {
        printf("Performing queued tasks for the given module\n");
        for(auto it = m_TasksToDoWhenModuleComesOnline.at(module).cbegin() ; it != m_TasksToDoWhenModuleComesOnline.at(module).cend() ; ++it)
        {
            (*it)();
        }
        m_TasksToDoWhenModuleComesOnline.erase(module);
    }
}

void ModuleExternalLinkGeneric::ExternalModuleRemoved(const CommsMACE::Resource &resource)
{
    throw std::runtime_error("Not Implimented");
}

std::string ModuleExternalLinkGeneric::createLog(const int &systemID)
{
    loggerCreated = true;
    std::string logname = this->loggingPath + "/ExternalLinkModule" + std::to_string(systemID) + ".txt";
    std::string loggerName = "ExternalLinkModule_" + std::to_string(systemID);
    char logNameArray[loggerName.size()+1];//as 1 char space for null is also required
    strcpy(logNameArray, loggerName.c_str());

    //initiate the logs
    size_t q_size = 8192; //queue size must be power of 2
    spdlog::set_async_mode(q_size,spdlog::async_overflow_policy::discard_log_msg,nullptr,std::chrono::seconds(2));
    mLog = spdlog::basic_logger_mt(logNameArray, logname);
    mLog->set_level(spdlog::level::debug);
    return loggerName;
}

void ModuleExternalLinkGeneric::TransmitMessage(const mace_message_t &msg, const OptionalParameter<MaceCore::ModuleCharacteristic> &target) const
{
    //broadcast
    if(target.IsSet() == false)
    {
        m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
        return;
    }

    // If the ID is zero, then we are sending message to a generic MACE instance
    // otherwise we are sending to an individual module
    if(target().ModuleID == 0)
    {
        CommsMACE::Resource r;
        r.Set<CommsMACE::MACE_INSTANCE_STR>(target().MaceInstance);
        m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg, r);
    }
    else
    {
        // Sending to an individual module.
        // Determine the type and send out on digimesh link appropriately.
        MaceCore::ModuleClasses type = this->getDataObject()->getModuleType(target());

        if(type == MaceCore::ModuleClasses::VEHICLE_COMMS)
        {
            CommsMACE::Resource r;
            r.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::VEHICLE_STR>(target().MaceInstance, target().ModuleID);
            m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg, r);
        }
        else if(type == MaceCore::ModuleClasses::GROUND_STATION)
        {
            CommsMACE::Resource r;
            r.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::GROUNDSTATION_STR>(target().MaceInstance, target().ModuleID);
            m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg, r);
        }
        else if(type == MaceCore::ModuleClasses::RTA)
        {
            CommsMACE::Resource r;
            r.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::RTA_STR>(target().MaceInstance, target().ModuleID);
            m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg, r);
        }
        else if(type == MaceCore::ModuleClasses::EXTERNAL_LINK)
        {
            CommsMACE::Resource r;
            r.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::EXTERNAL_LINK_STR>(target().MaceInstance, target().ModuleID);
            m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg, r);
        }
        else {
            throw std::runtime_error("Unknown target given");
        }
    }
}

std::vector<MaceCore::ModuleCharacteristic> ModuleExternalLinkGeneric::GetAllTargets() const
{
    return this->getDataObject()->GetAllRemoteModules();
}

MaceCore::ModuleCharacteristic ModuleExternalLinkGeneric::GetModuleFromMAVLINKVehicleID(int ID) const
{
    return this->getDataObject()->GetVehicleFromMAVLINKID(ID);
}

MaceCore::ModuleCharacteristic ModuleExternalLinkGeneric::GetHostKey() const
{
    MaceCore::ModuleCharacteristic key;
    key.ModuleID = 0;
    key.MaceInstance = this->getParentMaceInstanceID();

    return key;
}

std::tuple<int, int> ModuleExternalLinkGeneric::GetSysIDAndCompIDFromComponentKey(const MaceCore::ModuleCharacteristic &key) const
{
    return std::make_tuple(key.MaceInstance, key.ModuleID);
}

///////////////////////////////////////////////////////////////////////////////////////
/// The following are public virtual functions imposed from the Heartbeat Controller
/// Interface via callback functionality.
///////////////////////////////////////////////////////////////////////////////////////
void ModuleExternalLinkGeneric::cbiHeartbeatController_transmitCommand(const mace_heartbeat_t &heartbeat)
{
    mace_message_t msg;
    mace_msg_heartbeat_encode_chan(this->associatedSystemID,0,m_LinkChan,&msg,&heartbeat);
    TransmitMessage(msg);
}






//!
//! \brief New Mavlink message received over a link
//! \param linkName Name of link message received over
//! \param msg Message received
//!
void ModuleExternalLinkGeneric::MACEMessage(const std::string &linkName, const mace_message_t &message)
{
    UNUSED(linkName);
    MaceCore::ModuleCharacteristic sender;
    sender.MaceInstance = message.sysid;
    sender.ModuleID = message.compid;


    /// Check if the sender is a known source
    ///   If it's from a generic MACE instance then check if its a known instance
    ///   If it's from a specific module, check if its a known module
    bool knownSource = false;
    if(sender.ModuleID == 0 && this->getDataObject()->KnownMaceInstance(sender.MaceInstance))
    {
        knownSource = true;
    }

    if(sender.ModuleID != 0 && this->getDataObject()->HasModule(sender) == true)
    {
        knownSource = true;
    }


    if(knownSource == false)
    {
        printf("Received a %d message but sending Module is unknown, IGNORING. Mace: %d Module: %d\n", message.msgid, sender.MaceInstance, sender.ModuleID);
        this->RequestRemoteResources();
        return;
    }


    m_Controllers.ForEach<Controllers::IController<mace_message_t, MaceCore::ModuleCharacteristic>>([sender, message](Controllers::IController<mace_message_t, MaceCore::ModuleCharacteristic>* ptr) {
       ptr->ReceiveMessage(&message, sender);
    });

    for(auto it = m_TopicToControllers.cbegin() ; it != m_TopicToControllers.cend() ; ++it)
    {
        it->second->ReceiveMessage(&message, sender);
    }

    this->ParseForData(&message);
}



void ModuleExternalLinkGeneric::HeartbeatInfo(const MaceCore::ModuleCharacteristic &sender, const mace_heartbeat_t &heartbeatMSG)
{
    int systemID = heartbeatMSG.mavlinkID;

    CheckAndAddVehicle(sender, systemID);


    DataGenericItem::DataGenericItem_Heartbeat heartbeat;
    heartbeat.setAutopilot(static_cast<Data::AutopilotType>(heartbeatMSG.autopilot));
    heartbeat.setCompanion((heartbeatMSG.mace_companion>0)? true : false);
    heartbeat.setProtocol(static_cast<Data::CommsProtocol>(heartbeatMSG.protocol));
    heartbeat.setExecutionState(static_cast<Data::MissionExecutionState>(heartbeatMSG.mission_state));
    heartbeat.setType(static_cast<Data::SystemType>(heartbeatMSG.type));
    heartbeat.setMavlinkID(heartbeatMSG.mavlinkID);
    //heartbeat.setExecutionState(static_cast<Data::MissionExecutionState>(heartbeatMSG.missionState));
    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_Heartbeat> ptrHeartbeat = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_Heartbeat>(heartbeat);
    PublishVehicleData(sender, ptrHeartbeat);
}

//!
//! \brief ModuleExternalLinkGeneric::VehicleCommandMACEACK
//! \param linkName
//! \param systemID
//! \param cmdACK
//!
//Ken Fix This
//void ModuleExternalLinkGeneric::MACECommandACK(const std::string &linkName, const int &systemID, const mace_command_ack_t &cmdACK)
//{
//    m_CommandController->receivedCommandACK();
//    Data::CommandItemType cmdType = static_cast<Data::CommandItemType>(cmdACK.command);
//    Data::CommandACKType ackType = static_cast<Data::CommandACKType>(cmdACK.result);

//    std::string cmdString = Data::CommandItemTypeToString(cmdType);
//    std::string ackString = Data::CommandACKToString(ackType);

//    std::stringstream ss;
//    ss << "System " << systemID << " " << ackString << " command " << cmdString;
//    std::string newString = ss.str();

//    MaceCore::TopicDatagram topicDatagram;
//    DataGenericItem::DataGenericItem_Text newText;
//    newText.setSeverity(Data::StatusSeverityType::STATUS_INFO);
//    newText.setText(newString);
//    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_Text> ptrStatusText = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_Text>(newText);

//    m_VehicleDataTopic.SetComponent(ptrStatusText, topicDatagram);
//    //notify listeners of topic
//    ModuleExternalLinkGeneric::NotifyListenersOfTopic([&](MaceCore::IModuleTopicEvents* ptr){
//        ptr->NewTopicDataValues(this, m_VehicleDataTopic.Name(), systemID, MaceCore::TIME(), topicDatagram);
//    });

//}

void ModuleExternalLinkGeneric::PublishVehicleData(const MaceCore::ModuleCharacteristic &sender, const std::shared_ptr<Data::ITopicComponentDataObject> &component)
{
    //construct datagram
    MaceCore::TopicDatagram topicDatagram;
    m_VehicleDataTopic.SetComponent(component, topicDatagram);

    //notify listeners of topic
    ModuleExternalLinkGeneric::NotifyListenersOfTopic([&](MaceCore::IModuleTopicEvents* ptr){
        ptr->NewTopicDataValues(this, m_VehicleDataTopic.Name(), sender, MaceCore::TIME(), topicDatagram);
    });
}

void ModuleExternalLinkGeneric::PublishMissionData(const MaceCore::ModuleCharacteristic &sender, const std::shared_ptr<Data::ITopicComponentDataObject> &component)
{
    //construct datagram
    MaceCore::TopicDatagram topicDatagram;
    m_MissionDataTopic.SetComponent(component, topicDatagram);

    //notify listeners of topic
    ModuleExternalLinkGeneric::NotifyListenersOfTopic([&](MaceCore::IModuleTopicEvents* ptr){
        ptr->NewTopicDataValues(this, m_MissionDataTopic.Name(), sender, MaceCore::TIME(), topicDatagram);
    });
}

///////////////////////////////////////////////////////////////////////////////////////
/// The following are public virtual functions imposed from IModuleCommandExternalLink.
//////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
/// GENERAL VEHICLE COMMAND EVENTS: These are events that may have a direct
/// command and action sequence that accompanies the vheicle. Expect an
/// acknowledgement or an event to take place when calling these items.
////////////////////////////////////////////////////////////////////////////

void ModuleExternalLinkGeneric::Request_FullDataSync(const int &targetSystem, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    if(sender.IsSet() == false) {
        throw std::runtime_error("no sender given");
    }



    //This first segment causes all the topics to be republished
    mace_message_t msg;
    mace_vehicle_sync_t sync;
    sync.target_system = targetSystem;
    mace_msg_vehicle_sync_encode_chan(sender().MaceInstance, sender().ModuleID, m_LinkChan, &msg, &sync);

    if(targetSystem == 0)
    {
        TransmitMessage(msg);
    }
    else {
        MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(targetSystem);

        TransmitMessage(msg, target);

        m_Controllers.Retreive<ExternalLink::ControllerHome>()->Request(sender(), target);
    }
}

void ModuleExternalLinkGeneric::Command_SystemArm(const CommandItem::ActionArm &systemArm, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    int targetMavlinkSystemID = systemArm.getTargetSystem();

    if(targetMavlinkSystemID == 0)
    {
        m_Controllers.Retreive<ExternalLink::CommandARM>()->Broadcast(systemArm, sender());
    }
    else {
        MaceCore::ModuleCharacteristic target = getDataObject()->GetVehicleFromMAVLINKID(targetMavlinkSystemID);
        m_Controllers.Retreive<ExternalLink::CommandARM>()->Send(systemArm, sender(), target);
    }
}

void ModuleExternalLinkGeneric::Command_ChangeSystemMode(const CommandItem::ActionChangeMode &vehicleMode, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    std::cout<<"We are trying to change the mode in external link"<<std::endl;

    int targetMavlinkSystemID = vehicleMode.getTargetSystem();
    MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(targetMavlinkSystemID);
    m_Controllers.Retreive<ExternalLink::ControllerSystemMode>()->Send(vehicleMode, sender(), target);
}

void ModuleExternalLinkGeneric::Command_VehicleTakeoff(const CommandItem::SpatialTakeoff &vehicleTakeoff, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    int targetMavlinkSystemID = vehicleTakeoff.getTargetSystem();

    if(targetMavlinkSystemID == 0)
    {
        m_Controllers.Retreive<ExternalLink::CommandTakeoff>()->Broadcast(vehicleTakeoff, sender());
    }
    else {
        MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(targetMavlinkSystemID);
        m_Controllers.Retreive<ExternalLink::CommandTakeoff>()->Send(vehicleTakeoff, sender(), target);
    }
}

void ModuleExternalLinkGeneric::Command_Land(const CommandItem::SpatialLand &vehicleLand, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    int targetMavlinkSystemID = vehicleLand.getTargetSystem();

    if(targetMavlinkSystemID == 0)
    {
        m_Controllers.Retreive<ExternalLink::CommandLand>()->Broadcast(vehicleLand, sender());
    }
    else {
        MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(targetMavlinkSystemID);
        m_Controllers.Retreive<ExternalLink::CommandLand>()->Send(vehicleLand, sender(), target);
    }
}

void ModuleExternalLinkGeneric::Command_ReturnToLaunch(const CommandItem::SpatialRTL &vehicleRTL, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    if(vehicleRTL.getTargetSystem() == 0)
    {
        m_Controllers.Retreive<ExternalLink::CommandRTL>()->Broadcast(vehicleRTL, sender());
    }
    else {
        MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(vehicleRTL.getTargetSystem());
        m_Controllers.Retreive<ExternalLink::CommandRTL>()->Send(vehicleRTL, sender(), target);
    }
}

void ModuleExternalLinkGeneric::Command_MissionState(const CommandItem::ActionMissionCommand &command, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(command.getTargetSystem());

    if(target.ModuleID == 0)
    {
        m_Controllers.Retreive<ExternalLink::CommandMissionItem>()->Broadcast(command, sender());
    }
    else {
        m_Controllers.Retreive<ExternalLink::CommandMissionItem>()->Send(command, sender(), target);
    }
}

void ModuleExternalLinkGeneric::Command_IssueGeneralCommand(const std::shared_ptr<CommandItem::AbstractCommandItem> &command)
{
    UNUSED(command);
}

void ModuleExternalLinkGeneric::Command_EmitHeartbeat(const CommandItem::SpatialTakeoff &heartbeat)
{

}

/////////////////////////////////////////////////////////////////////////////
/// GENERAL HOME EVENTS: These events are related to establishing or setting
/// a home position. It should be recognized that the first mission item in a
/// mission queue should prepend this position. Just the way ardupilot works.
/////////////////////////////////////////////////////////////////////////////

void ModuleExternalLinkGeneric::Command_GetHomePosition(const int &vehicleID, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    std::cout<<"External link module saw a get home position request"<<std::endl;

    MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(vehicleID);

    m_Controllers.Retreive<ExternalLink::ControllerHome>()->Request(sender(), target);
}

void ModuleExternalLinkGeneric::Command_SetHomePosition(const CommandItem::SpatialHome &systemHome, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(systemHome.getTargetSystem());

    m_Controllers.Retreive<ExternalLink::ControllerHome>()->Send(systemHome, sender(), target);
}

/////////////////////////////////////////////////////////////////////////
/// GENERAL MISSION EVENTS: This is implying for auto mode of the vehicle.
/// This functionality may be pertinent for vehicles not containing a
/// direct MACE hardware module.
/////////////////////////////////////////////////////////////////////////

void ModuleExternalLinkGeneric::Command_UploadMission(const MissionItem::MissionList &missionList)
{
    MissionItem::MissionList::MissionListStatus status = missionList.getMissionListStatus();

    MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(missionList.getVehicleID());

    if(status.state == MissionItem::MissionList::COMPLETE)
    {
        throw std::runtime_error("Not Implimented");
        //m_Controllers.Retreive<ExternalLink::ControllerMission>()->UploadMission(missionList, target);
    }
}

void ModuleExternalLinkGeneric::Command_GetMission(const MissionItem::MissionKey &key, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    if(sender.IsSet() == false) {
        throw std::runtime_error("no sender given");
    }

    MaceCore::ModuleCharacteristic target = this->getDataObject()->GetVehicleFromMAVLINKID(key.m_systemID);

    UNUSED(key);
    m_Controllers.Retreive<ExternalLink::ControllerMission>()->RequestMission(key, sender(), target);
}


void ModuleExternalLinkGeneric::Command_RequestBoundaryDownload(const std::tuple<MaceCore::ModuleCharacteristic, uint8_t> &data, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    std::cout << "External link is making a request to a remote instance" << std::endl;

    if(sender.IsSet() == false)
    {
        throw std::runtime_error("Sender must be set when requesting a boundary download on external link");
    }

    MaceCore::ModuleCharacteristic target = std::get<0>(data);
    uint8_t ID = std::get<1>(data);

    m_Controllers.Retreive<ExternalLink::ControllerBoundary>()->RequestBoundary(ID, sender(), target);
}

void ModuleExternalLinkGeneric::Command_SetCurrentMission(const MissionItem::MissionKey &key)
{
    //    mace_message_t msg;
    //    mace_mission_set_current_t request;
    //    request.
    //    request.mission_creator = key.m_creatorID;
    //    request.mission_id = key.m_missionID;
    //    request.mission_type = (uint8_t)key.m_missionType;
    //    request.target_system = key.m_systemID;
    //    mace_msg_mission_set_current_encode_chan(associatedSystemID,0,m_LinkChan,&msg,&request);
    //    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
}

void ModuleExternalLinkGeneric::Command_GetCurrentMission(const int &targetSystem)
{
    UNUSED(targetSystem);

}

void ModuleExternalLinkGeneric::Command_ClearCurrentMission(const int &targetSystem)
{
    UNUSED(targetSystem);
}

void ModuleExternalLinkGeneric::Command_GetOnboardAuto(const int &targetSystem)
{
    UNUSED(targetSystem);

}

void ModuleExternalLinkGeneric::Command_ClearOnboardAuto(const int &targetSystem)
{
    UNUSED(targetSystem);
}

void ModuleExternalLinkGeneric::Command_GetOnboardGuided(const int &targetSystem)
{
    UNUSED(targetSystem);
}

void ModuleExternalLinkGeneric::Command_ClearOnboardGuided(const int &targetSystem)
{
    UNUSED(targetSystem);
}


//!
//! \brief A new boundary item is available. Notify all known mace instances
//! \param key Local key of boundary. Identifies the boundary on the host machine
//! \param sender Module that generated the boundary
//!
void ModuleExternalLinkGeneric::NewlyAvailableBoundary(const uint8_t &boundaryKey, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    BoundaryNotificationData newBoundary;
    this->getDataObject()->getCharactersticFromIdentifier(boundaryKey, newBoundary.characteristic);
    newBoundary.uniqueIdentifier = boundaryKey;

    std::vector<MaceCore::ModuleCharacteristic> modules = this->getDataObject()->GetAllRemoteModules();

    std::set<uint8_t> uniqueInstances;
    for(auto it = modules.cbegin() ; it != modules.cend() ; ++it)
    {
        if(uniqueInstances.find(it->MaceInstance) == uniqueInstances.cend())
        {
            uniqueInstances.insert(it->MaceInstance);

            ///////////////////
            /// broadcast to instance
            uint8_t remoteMACEInstance = it->MaceInstance;

            MaceCore::ModuleCharacteristic target;
            target.MaceInstance = remoteMACEInstance;
            target.ModuleID = 0;

            m_Controllers.Retreive<ExternalLink::ControllerBoundary>()->Send(newBoundary, sender(), target);
        }
    }
}

void ModuleExternalLinkGeneric::NewlyAvailableOnboardMission(const MissionItem::MissionKey &key, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    std::vector<MaceCore::ModuleCharacteristic> modules = this->getDataObject()->GetAllRemoteModules();

    std::set<uint8_t> uniqueInstances;
    for(auto it = modules.cbegin() ; it != modules.cend() ; ++it)
    {
        if(uniqueInstances.find(it->MaceInstance) == uniqueInstances.cend())
        {
            uniqueInstances.insert(it->MaceInstance);

            ///////////////////
            /// broadcast to instance
            uint8_t remoteMACEInstance = it->MaceInstance;

            MaceCore::ModuleCharacteristic target;
            target.MaceInstance = remoteMACEInstance;
            target.ModuleID = 0;

            /// only send if the remote mace instance is known
            /// This is needed because the module may of been notified, but the resouce-indiation message for the instance itself hasn't made it yet
            CommsMACE::Resource r;
            r.Add(CommsMACE::MACE_INSTANCE_STR, remoteMACEInstance);
            if(this->m_LinkMarshaler->HasResource(m_LinkName, r) == true)
            {
                m_Controllers.Retreive<ExternalLink::ControllerMission>()->NotifyOfMission(key, sender(), target);
            }
            else
            {
                printf("MACE Instance is unknown, Requesting for remote resources\n");

                this->m_LinkMarshaler->RequestRemoteResources(m_LinkName, {r});
            }
        }
    }

    /*
    mace_new_onboard_mission_t mission;
    mission.mission_creator = key.m_creatorID;
    mission.mission_id = key.m_missionID;
    mission.mission_type = (uint8_t)key.m_missionType;
    mission.mission_system = key.m_systemID;
    mission.mission_state = (uint8_t)key.m_missionState;

    mace_message_t msg;
    mace_msg_new_onboard_mission_encode_chan(sender().MaceInstance, sender().ModuleID, m_LinkChan,&msg,&mission);

    printf("Broadcasting Mission Notification\n");

    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
    */
}

void ModuleExternalLinkGeneric::NewlyAvailableHomePosition(const CommandItem::SpatialHome &home, const OptionalParameter<MaceCore::ModuleCharacteristic> &sender)
{
    m_Controllers.Retreive<ExternalLink::ControllerHome>()->Broadcast(home, sender());
}

//Ken FIX THIS: I dont know if I should pass the pertinent systemID with the key
void ModuleExternalLinkGeneric::NewlyAvailableMissionExeState(const MissionItem::MissionKey &key)
{
    MissionItem::MissionList list;
    bool validity = this->getDataObject()->getMissionList(key,list);
    if(validity)
    {
        mace_mission_exe_state_t state;
        Data::MissionExecutionState missionState = list.getMissionExeState();
        state.mission_creator = key.m_creatorID;
        state.mission_id = key.m_missionID;
        state.mission_state = (uint8_t)missionState;
        state.mission_system = key.m_systemID;
        state.mission_type = (uint8_t)key.m_missionType;

        mace_message_t msg;
        mace_msg_mission_exe_state_encode_chan(key.m_systemID,0,m_LinkChan,&msg,&state);
        m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
    }
}

void ModuleExternalLinkGeneric::NewlyAvailableModule(const MaceCore::ModuleCharacteristic &module, const MaceCore::ModuleClasses &type)
{
    CommsMACE::Resource MACEInstanceResource;
    MACEInstanceResource.Set<CommsMACE::MACE_INSTANCE_STR>(module.MaceInstance);
    if(m_LinkMarshaler->HasResource(m_LinkName, MACEInstanceResource) == false)
    {
        m_LinkMarshaler->AddResource(m_LinkName, MACEInstanceResource);
    }


    CommsMACE::Resource resource;
    if(type == MaceCore::ModuleClasses::VEHICLE_COMMS)
    {
        resource.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::VEHICLE_STR>(module.MaceInstance, module.ModuleID);
    }
    if(type == MaceCore::ModuleClasses::GROUND_STATION)
    {
        resource.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::GROUNDSTATION_STR>(module.MaceInstance, module.ModuleID);
    }
    if(type == MaceCore::ModuleClasses::RTA)
    {
        resource.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::RTA_STR>(module.MaceInstance, module.ModuleID);
    }
    if(type == MaceCore::ModuleClasses::EXTERNAL_LINK)
    {
        resource.Set<CommsMACE::MACE_INSTANCE_STR, CommsMACE::EXTERNAL_LINK_STR>(module.MaceInstance, module.ModuleID);
    }


    m_LinkMarshaler->AddResource(m_LinkName, resource);

    if(airborneInstance)
    {
        this->associatedSystemID = module.ModuleID;

        //this function should always be called by an external link connected to ground for now
        //KEN this is a hack...but it will function for now
        ModuleExternalLinkGeneric::NotifyListeners([&](MaceCore::IModuleEventsExternalLink* ptr){
            ptr->ExternalEvent_UpdateRemoteID(this, 254);
        });
    }
}

void ModuleExternalLinkGeneric::ReceivedMissionACK(const MissionItem::MissionACK &ack)
{
    mace_mission_ack_t missionACK;
    MissionItem::MissionKey key = ack.getMissionKey();
    missionACK.cur_mission_state = (uint8_t)ack.getNewMissionState();
    missionACK.mission_creator = key.m_creatorID;
    missionACK.mission_id = key.m_missionID;
    missionACK.mission_result = (uint8_t)ack.getMissionResult();
    missionACK.mission_system = key.m_systemID;
    missionACK.mission_type = (uint8_t)key.m_missionType;
    missionACK.prev_mission_state = (uint8_t)key.m_missionState;

    mace_message_t msg;
    mace_msg_mission_ack_encode_chan(associatedSystemID,0,m_LinkChan,&msg,&missionACK);
    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
}


//!
//! \brief New non-spooled topic given
//!
//! NonSpooled topics send their data immediatly.
//! \param topicName Name of stopic
//! \param sender Module that sent topic
//! \param data Data for topic
//! \param target Target module (or broadcasted)
//!
void ModuleExternalLinkGeneric::NewTopicData(const std::string &topicName, const MaceCore::ModuleCharacteristic &sender, const MaceCore::TopicDatagram &data, const OptionalParameter<MaceCore::ModuleCharacteristic> &target)
{

    if(this->m_TopicToControllers.find(topicName) == m_TopicToControllers.cend())
    {
        throw std::runtime_error("Attempting to send a topic that external link has no knowledge of");
    }

    Controllers::IController<mace_message_t, MaceCore::ModuleCharacteristic> *controller = m_TopicToControllers.at(topicName);
    if(target.IsSet())
    {
        if(controller->ContainsAction(Controllers::Actions::SEND) == false)
        {
            throw std::runtime_error("Attempting to send a topic to a controller that has no send action");
        }
        Controllers::IActionSend<MaceCore::TopicDatagram, MaceCore::ModuleCharacteristic> *sendAction = dynamic_cast<Controllers::IActionSend<MaceCore::TopicDatagram, MaceCore::ModuleCharacteristic>*>(controller);
        sendAction->Send(data, sender, target());
    }
    else {
        if(controller->ContainsAction(Controllers::Actions::BROADCAST) == false)
        {
            throw std::runtime_error("Attempting to broadcast a topic to a controller that has no broadcast action");
        }
        Controllers::IActionBroadcast<MaceCore::TopicDatagram, MaceCore::ModuleCharacteristic> *sendAction = dynamic_cast<Controllers::IActionBroadcast<MaceCore::TopicDatagram, MaceCore::ModuleCharacteristic>*>(controller);
        sendAction->Broadcast(data, sender);
    }
}


//!
//! \brief New Spooled topic given
//!
//! Spooled topics are stored on the core's datafusion.
//! This method is used to notify other modules that there exists new data for the given components on the given module.
//! \param topicName Name of topic given
//! \param sender Module that sent topic
//! \param componentsUpdated Components in topic that where updated
//! \param target Target moudle (or broadcast)
//!
void ModuleExternalLinkGeneric::NewTopicSpooled(const std::string &topicName, const MaceCore::ModuleCharacteristic &sender, const std::vector<std::string> &componentsUpdated, const OptionalParameter<MaceCore::ModuleCharacteristic> &target)
{
    int senderID = this->getDataObject()->getMavlinkIDFromModule(sender);
    if(airborneInstance)
    {

        //In relevance to the external link module, the module when receiving a new topic should pack that up for transmission
        //to other instances of MACE
        //example read of vehicle data
        if(topicName == m_VehicleDataTopic.Name())
        {
            //get latest datagram from mace_data
            MaceCore::TopicDatagram read_topicDatagram = this->getDataObject()->GetCurrentTopicDatagram(m_VehicleDataTopic.Name(), sender);
            //example of how to get data and parse through the components that were updated
            for(size_t i = 0 ; i < componentsUpdated.size() ; i++) {
                if(componentsUpdated.at(i) == DataGenericItemTopic::DataGenericItemTopic_Heartbeat::Name()) {
                    if(!loggerCreated)
                        createLog(senderID);
                    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_Heartbeat> component = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_Heartbeat>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == DataGenericItemTopic::DataGenericItemTopic_GPS::Name()) {
                    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_GPS> component = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_GPS>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == DataGenericItemTopic::DataGenericItemTopic_FlightMode::Name()) {
                    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_FlightMode> component = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_FlightMode>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == DataGenericItemTopic::DataGenericItemTopic_SystemArm::Name()) {
                    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_SystemArm> component = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_SystemArm>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == DataStateTopic::StateAttitudeTopic::Name()) {
                    std::shared_ptr<DataStateTopic::StateAttitudeTopic> component = std::make_shared<DataStateTopic::StateAttitudeTopic>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg_Full(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == DataStateTopic::StateGlobalPositionTopic::Name()) {
                    std::shared_ptr<DataStateTopic::StateGlobalPositionTopic> component = std::make_shared<DataStateTopic::StateGlobalPositionTopic>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == DataGenericItemTopic::DataGenericItemTopic_Battery::Name()) {
                    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_Battery> component = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_Battery>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == DataGenericItemTopic::DataGenericItemTopic_Text::Name()) {
                    std::shared_ptr<DataGenericItemTopic::DataGenericItemTopic_Text> component = std::make_shared<DataGenericItemTopic::DataGenericItemTopic_Text>();
                    m_VehicleDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
            }
        } //end of vehicle data topic
        else if(topicName == m_MissionDataTopic.Name())
        {
            for(size_t i = 0 ; i < componentsUpdated.size() ; i++) {
                //get latest datagram from mace_data
                MaceCore::TopicDatagram read_topicDatagram = this->getDataObject()->GetCurrentTopicDatagram(m_MissionDataTopic.Name(), sender);
                if(componentsUpdated.at(i) == MissionTopic::MissionItemCurrentTopic::Name()) {
                    std::shared_ptr<MissionTopic::MissionItemCurrentTopic> component = std::make_shared<MissionTopic::MissionItemCurrentTopic>();
                    m_MissionDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
                else if(componentsUpdated.at(i) == MissionTopic::VehicleTargetTopic::Name()) {
                    std::shared_ptr<MissionTopic::VehicleTargetTopic> component = std::make_shared<MissionTopic::VehicleTargetTopic>();
                    m_MissionDataTopic.GetComponent(component, read_topicDatagram);
                    mace_message_t msg = component->getMACEMsg(sender.MaceInstance, sender.ModuleID, m_LinkChan);
                    m_LinkMarshaler->SendMACEMessage<mace_message_t>(m_LinkName, msg);
                }
            }
        }
    }//end of if airborne instance

}



//!
//! \brief Procedure to perform when a new MACE instance is added.
//!
//! This method will consult with data object and send out any relevant data to the remote MACE instance
//!
//! \param MaceInstanceID ID of new mace instance.
//!
void ModuleExternalLinkGeneric::NewExternalMaceInstance(uint8_t maceInstanceID)
{
    printf("New MACE instance with ID of %d detected\n", maceInstanceID);

    MaceCore::ModuleCharacteristic target;
    target.MaceInstance = maceInstanceID;
    target.ModuleID = 0;

    /// Iterate over every local vehicle and do tasks on them
    std::vector<int> localVehicles = this->getDataObject()->GetLocalVehicles();
    for(auto localVehicle_it = localVehicles.cbegin() ; localVehicle_it != localVehicles.cend() ; ++localVehicle_it)
    {
        /// Local vehicle
        int vehicleID = *localVehicle_it;
        MaceCore::ModuleCharacteristic sender = this->getDataObject()->GetVehicleFromMAVLINKID(vehicleID);


        /// What to send about the new vehicle to the new MACE instance
        auto func = [this, vehicleID, sender, target]()
        {
            std::vector<MissionItem::MissionKey> internalMissions = this->getDataObject()->getMissionKeysForVehicle(vehicleID);

            for(auto it = internalMissions.cbegin() ; it != internalMissions.cend() ; ++it)
            {
                m_Controllers.Retreive<ExternalLink::ControllerMission>()->NotifyOfMission(*it, sender, target);
            }
        };


        /// Check if this can execute now or if we need to wait until we learn about the module in question.
        if(this->getDataObject()->HasModule(sender) == true)
        {
            func();
        }
        else
        {
            printf("Module is not online yet, add as tasks to perform when online\n");
            if(m_TasksToDoWhenModuleComesOnline.find(sender) == m_TasksToDoWhenModuleComesOnline.cend())
            {
                m_TasksToDoWhenModuleComesOnline.insert({sender, {}});
            }
            m_TasksToDoWhenModuleComesOnline[sender].push_back(func);
        }
    }
}


//!
//! \brief Check if the given systemID is known. If unknown then do steps to add the vehicle to this MACE instance
//! \param sender
//! \param systemID
//!
void ModuleExternalLinkGeneric::CheckAndAddVehicle(const MaceCore::ModuleCharacteristic &sender, int systemID)
{
    if(systemIDMap.find(systemID) == systemIDMap.end())
    {
        //this function should always be called by an external link connected to ground for now
        //KEN this is a hack...but it will function for now
        ModuleExternalLinkGeneric::NotifyListeners([&](MaceCore::IModuleEventsExternalLink* ptr){
            ptr->ExternalEvent_UpdateRemoteID(this, systemID);
        });

        std::string loggerName = createLog(systemID);
        systemIDMap.insert({systemID,0});

        //The system has yet to have communicated through this module
        //We therefore have to notify the core that there is a new vehicle
        ModuleExternalLinkGeneric::NotifyListeners([&](MaceCore::IModuleEventsExternalLink* ptr){
            ptr->Events_NewVehicle(this, systemID, sender);
        });

        //Request_FullDataSync(systemID);
    }
}