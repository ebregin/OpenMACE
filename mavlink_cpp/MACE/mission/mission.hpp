/** @file
 *	@brief MAVLink comm protocol generated from mission.xml
 *	@see http://mavlink.org
 */

#pragma once

#include <array>
#include <cstdint>
#include <sstream>

#ifndef MAVLINK_STX
#define MAVLINK_STX 253
#endif

#include "../message.hpp"

namespace mavlink {
namespace mission {

/**
 * Array of msg_entry needed for @p mavlink_parse_char() (trought @p mavlink_get_msg_entry())
 */
constexpr std::array<mavlink_msg_entry_t, 21> MESSAGE_ENTRIES {{ {100, 132, 5, 0, 0, 0}, {101, 18, 7, 1, 2, 0}, {102, 87, 7, 0, 0, 0}, {103, 135, 3, 0, 0, 0}, {104, 84, 5, 0, 0, 0}, {105, 165, 8, 1, 2, 0}, {106, 3, 8, 1, 2, 0}, {107, 49, 41, 1, 32, 0}, {108, 4, 7, 3, 4, 5}, {109, 168, 7, 3, 4, 5}, {110, 224, 4, 0, 0, 0}, {111, 156, 5, 0, 0, 0}, {112, 47, 7, 0, 0, 0}, {113, 54, 7, 0, 0, 0}, {114, 34, 4, 1, 0, 0}, {115, 55, 5, 0, 0, 0}, {116, 36, 1, 1, 0, 0}, {117, 149, 53, 0, 0, 0}, {118, 85, 53, 1, 52, 0}, {119, 21, 2, 1, 0, 0}, {120, 12, 18, 0, 0, 0} }};

//! MAVLINK VERSION
constexpr auto MAVLINK_VERSION = 3;


// ENUM DEFINITIONS


/** @brief  The ROI (region of interest) for the vehicle. This can be
                be used by the vehicle for camera/vehicle attitude alignment (see
                MAV_CMD_NAV_ROI). */
enum class MAV_ROI
{
    NONE=0, /* No region of interest. | */
    WPNEXT=1, /* Point toward next MISSION. | */
    WPINDEX=2, /* Point toward given MISSION. | */
    LOCATION=3, /* Point toward fixed location. | */
    TARGET=4, /* Point toward of given id. | */
};

//! MAV_ROI ENUM_END
constexpr auto MAV_ROI_ENUM_END = 5;

/** @brief result in a mavlink mission ack */
enum class MAV_MISSION_RESULT : uint8_t
{
    ACCEPTED=0, /* mission accepted OK | */
    ERROR=1, /* generic error / not accepting mission commands at all right now | */
    UNSUPPORTED_FRAME=2, /* coordinate frame is not supported | */
    UNSUPPORTED=3, /* command is not supported | */
    NO_SPACE=4, /* mission item exceeds storage space | */
    INVALID=5, /* one of the parameters has an invalid value | */
    INVALID_PARAM1=6, /* param1 has an invalid value | */
    INVALID_PARAM2=7, /* param2 has an invalid value | */
    INVALID_PARAM3=8, /* param3 has an invalid value | */
    INVALID_PARAM4=9, /* param4 has an invalid value | */
    INVALID_PARAM5_X=10, /* x/param5 has an invalid value | */
    INVALID_PARAM6_Y=11, /* y/param6 has an invalid value | */
    INVALID_PARAM7=12, /* param7 has an invalid value | */
    INVALID_SEQUENCE=13, /* received waypoint out of sequence | */
    DENIED=14, /* not accepting any mission commands from this communication partner | */
    DOES_NOT_EXIST=15, /* The requested mission with the associated key does not exist. | */
};

//! MAV_MISSION_RESULT ENUM_END
constexpr auto MAV_MISSION_RESULT_ENUM_END = 16;

/** @brief Type of mission items being requested/sent in mission protocol. */
enum class MAV_MISSION_TYPE
{
    AUTO=0, /* Items are mission commands for main auto mission. | */
    GUIDED=1, /* Items are mission commands for guided mission. | */
    ROI=2, /* Items are regions of interest that the vehicle should visit in a guided mission. | */
    FENCE=3, /* Specifies GeoFence area(s). Items are MAV_CMD_FENCE_ GeoFence items. | */
    RALLY=4, /* Specifies the rally points for the vehicle. Rally points are alternative RTL points. Items are MAV_CMD_RALLY_POINT rally point items. | */
    ALL=255, /* Only used in MISSION_CLEAR_ALL to clear all mission types. | */
};

//! MAV_MISSION_TYPE ENUM_END
constexpr auto MAV_MISSION_TYPE_ENUM_END = 256;

/** @brief Type of mission items being requested/sent in mission protocol. */
enum class MAV_MISSION_STATE
{
    CURRENT=0, /* Items are mission commands for main auto mission and are received via the autopilot and aircraft module. | */
    ONBOARD=1, /* Items are mission ready and have been acknowledged by the aircraft module for processing. | */
    PROPOSED=2, /* Items are mission ready but have been generated by a module not related to the aircraft and need to be pushed towards the appropriate aircraft module. | */
    RECEIVED=3, /* Items have been distributed and received by the appropriate MACE instance connected to the associated module. They however have yet to be marked as current or onboard. Often this state is reflected via communication between external link modules. | */
    OUTDATED=4, /* Items were once relevant to the system have been since replaced with newer current missions. | */
};

//! MAV_MISSION_STATE ENUM_END
constexpr auto MAV_MISSION_STATE_ENUM_END = 5;


} // namespace mission
} // namespace mavlink

// MESSAGE DEFINITIONS
#include "./mavlink_msg_new_onboard_mission.hpp"
#include "./mavlink_msg_new_proposed_mission.hpp"
#include "./mavlink_msg_mission_ack.hpp"
#include "./mavlink_msg_mission_request_list_generic.hpp"
#include "./mavlink_msg_mission_request_list.hpp"
#include "./mavlink_msg_mission_count.hpp"
#include "./mavlink_msg_mission_request_item.hpp"
#include "./mavlink_msg_mission_item.hpp"
#include "./mavlink_msg_mission_request_partial_list.hpp"
#include "./mavlink_msg_mission_write_partial_list.hpp"
#include "./mavlink_msg_starting_current_mission.hpp"
#include "./mavlink_msg_mission_set_current.hpp"
#include "./mavlink_msg_mission_item_current.hpp"
#include "./mavlink_msg_mission_item_reached.hpp"
#include "./mavlink_msg_mission_clear.hpp"
#include "./mavlink_msg_mission_exe_state.hpp"
#include "./mavlink_msg_mission_request_home.hpp"
#include "./mavlink_msg_home_position.hpp"
#include "./mavlink_msg_set_home_position.hpp"
#include "./mavlink_msg_home_position_ack.hpp"
#include "./mavlink_msg_guided_target_stats.hpp"

// base include
#include "../common/common.hpp"
