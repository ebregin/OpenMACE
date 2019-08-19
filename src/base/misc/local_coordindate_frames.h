#ifndef LOCAL_COORDINDATE_FRAMES_H
#define LOCAL_COORDINDATE_FRAMES_H

#include <string>
#include <stdexcept>

#include "mace.h"

namespace mace {

#define LOCAL_FRAMES CF_LOCAL_UNKNOWN, /* Local coordinate frame is unknown | */ \
CF_LOCAL_NED, /* Local coordinate frame, Z-up (x: north, y: east, z: down). | */ \
CF_LOCAL_ENU, /* Local coordinate frame, Z-down (x: east, y: north, z: up) | */ \
CF_LOCAL_OFFSET_NED, /* Offset to the current local frame. Anything expressed in this frame should be added to the current local frame position. | */ \
CF_BODY_NED, /* Setpoint in body NED frame. This makes sense if all position control is externalized - e.g. useful to command 2 m/s^2 acceleration to the right. | */ \
CF_BODY_ENU /* Offset in body NED frame. This makes sense if adding setpoints to the current flight path, to avoid an obstacle - e.g. useful to command 2 m/s^2 acceleration to the east. | */


enum class CartesianFrameTypes: uint8_t{
    LOCAL_FRAMES
};

inline std::string CartesianFrameToString(const CartesianFrameTypes &frame) {

    switch (frame) {
    case CartesianFrameTypes::CF_LOCAL_NED:
        return "REF_CART_LOCAL_NED";
    case CartesianFrameTypes::CF_LOCAL_ENU:
        return "REF_CART_LOCAL_ENU";
    case CartesianFrameTypes::CF_LOCAL_OFFSET_NED:
        return "REF_CART_LOCAL_OFFSET";
    case CartesianFrameTypes::CF_BODY_NED:
        return "REF_CART_VEHICLE_NED";
    case CartesianFrameTypes::CF_BODY_ENU:
        return "REF_CART_VEHICLE_ENU";
    case CartesianFrameTypes::CF_LOCAL_UNKNOWN:
        return "REF_CART_LOCAL_UNKNOWN";
    default:
        throw std::runtime_error("Unknown altitude reference seen");
    }
}

inline CartesianFrameTypes CartesianFrameFromString(const std::string &str) {
    if(str == "REF_CART_LOCAL_NED")
        return CartesianFrameTypes::CF_LOCAL_NED;
    if(str == "REF_CART_LOCAL_ENU")
        return CartesianFrameTypes::CF_LOCAL_ENU;
    if(str == "REF_CART_LOCAL_OFFSET")
        return CartesianFrameTypes::CF_LOCAL_OFFSET_NED;
    if(str == "REF_CART_VEHICLE_NED")
        return CartesianFrameTypes::CF_BODY_NED;
    if(str == "REF_CART_VEHICLE_ENU")
        return CartesianFrameTypes::CF_BODY_ENU;
    if(str == "REF_CART_LOCAL_UNKNOWN")
        return CartesianFrameTypes::CF_LOCAL_UNKNOWN;
    throw std::runtime_error("Unknown altitude reference seen");
}

} //end of namespace mace

#endif // LOCAL_COORDINDATE_FRAMES_H
