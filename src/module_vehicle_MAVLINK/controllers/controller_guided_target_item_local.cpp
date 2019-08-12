#include "controller_guided_target_item_local.h"
#include "base/pose/pose_components.h"

/*
MAV_FRAME_LOCAL_NED
Positions are relative to the vehicle’s EKF Origin in NED frame

I.e x=1,y=2,z=3 is 1m North, 2m East and 3m Down from the origin

The EKF origin is the vehicle’s location when it first achieved a good position estimate

Velocity are in NED frame

MAV_FRAME_LOCAL_OFFSET_NED
Positions are relative to the vehicle’s current position

I.e. x=1,y=2,z=3 is 1m North, 2m East and 3m below the current position.

Velocity are in NED frame.

MAV_FRAME_BODY_OFFSET_NED
Positions are relative to the vehicle’s current position and heading

I.e x=1,y=2,z=3 is 1m forward, 2m right and 3m Down from the current position

Velocities are relative to the current vehicle heading. Use this to specify the speed forward, right and down (or the opposite if you use negative values).

MAV_FRAME_BODY_NED
Positions are relative to the EKF Origin in NED frame

I.e x=1,y=2,z=3 is 1m North, 2m East and 3m Down from the origin

Velocities are relative to the current vehicle heading. Use this to specify the speed forward, right and down (or the opposite if you use negative values).
*/

namespace MAVLINKVehicleControllers {

template <>
void ControllerGuidedTargetItem_Local<TargetControllerStructLocal>::FillTargetItem(const TargetControllerStructLocal &targetStruct, mavlink_set_position_target_local_ned_t &mavlinkItem)
{
    double power = pow(10,7);
    uint16_t bitArray = 65535;
    command_target::DynamicTarget currentTarget = targetStruct.target;
    mavlinkItem.coordinate_frame = MAV_FRAME_LOCAL_NED;

    //This handles the packing of the position components
    if(currentTarget.getPosition()->isAnyPositionValid())
    {
        if(currentTarget.getPosition()->is2D())
        {
            mace::pose::CartesianPosition_2D* castPosition = currentTarget.getPosition()->positionAs<mace::pose::CartesianPosition_2D>();
            if(castPosition->hasXBeenSet())
                mavlinkItem.x = static_cast<float>(castPosition->getYPosition() * power); bitArray = (bitArray & (~1));
            if(castPosition->hasYBeenSet())
                mavlinkItem.y = static_cast<float>(castPosition->getXPosition() * power); bitArray = (bitArray & (~2));
        }
        else if(currentTarget.getPosition()->is3D())
        {
            mace::pose::CartesianPosition_3D* castPosition = currentTarget.getPosition()->positionAs<mace::pose::CartesianPosition_3D>();
            if(castPosition->hasXBeenSet())
                mavlinkItem.x = static_cast<float>(castPosition->getXPosition()); bitArray = (bitArray & (~1));
            if(castPosition->hasYBeenSet())
                mavlinkItem.y = static_cast<float>(castPosition->getYPosition()); bitArray = (bitArray & (~2));
            if(castPosition->hasZBeenSet())
                mavlinkItem.z = static_cast<float>(castPosition->getAltitude());  bitArray = (bitArray & (~4));
        }
    }

    //This handles the packing of the velocity components
    if(currentTarget.getVelocity()->isAnyVelocityValid())
    {
        if(currentTarget.getVelocity()->is2D())
        {
            mace::pose::Cartesian_Velocity2D* castVelocity = currentTarget.getVelocity()->velocityAs<mace::pose::Cartesian_Velocity2D>();
            if(castVelocity->hasXBeenSet())
                mavlinkItem.vx = static_cast<float>(castVelocity->getXVelocity()); bitArray = (bitArray & (~8));
            if(castVelocity->hasYBeenSet()())
                mavlinkItem.vy = static_cast<float>(castVelocity->getYVelocity()); bitArray = (bitArray & (~16));
        }
        else if(currentTarget.getVelocity()->is3D())
        {
            mace::pose::Cartesian_Velocity3D* castVelocity = currentTarget.getVelocity()->velocityAs<mace::pose::Cartesian_Velocity3D>();
            if(castVelocity->hasXBeenSet())
                mavlinkItem.vx = static_cast<float>(castVelocity->getXVelocity()); bitArray = (bitArray & (~8));
            if(castVelocity->hasYBeenSet()())
                mavlinkItem.vy = static_cast<float>(castVelocity->getYVelocity()); bitArray = (bitArray & (~16));
            if(castVelocity->hasZBeenSet()())
                mavlinkItem.vz = static_cast<float>(castVelocity->getZVelocity()); bitArray = (bitArray & (~32));
        }
    }

    if(currentTarget.getYaw()->isYawDimensionSet())
    {
        mace::pose::Rotation_2D* castRotation = currentTarget.getYaw();
        mavlinkItem.yaw = castRotation->getPhi(); (bitArray & (~1024));
    }

    if(currentTarget.getYawRate()->isYawDimensionSet())
    {
        mace::pose::Rotation_2D* castRotation = currentTarget.getYaw();
        mavlinkItem.yaw_rate = castRotation->getPhi(); (bitArray & (~2048));
    }
    mavlinkItem.type_mask = bitArray;
    mavlinkItem.type_mask = bitArray;
}

}// end of namespace MAVLINKVehicleControllers
