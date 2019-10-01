#include "controller_guided_target_item_attitude.h"

namespace MAVLINKUXVControllers {

template<>
void ControllerGuidedTargetItem_Attitude<command_item::Action_DynamicTarget>::FillTargetItem(const command_item::Action_DynamicTarget &command, mavlink_set_position_target_global_int_t &mavlinkItem)
{
    double power = pow(10,7);
    uint16_t bitArray = 3583; // first let us assume that they are all

    command_target::DynamicTarget currentTarget = command.getDynamicTarget();

    mavlinkItem.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;

    //This handles the packing of the position components
    if((currentTarget.getPosition() != nullptr) && (currentTarget.getPosition()->isAnyPositionValid()))
    {
        if(currentTarget.getPosition()->is2D())
        {
            mace::pose::GeodeticPosition_2D* castPosition = currentTarget.getPosition()->positionAs<mace::pose::GeodeticPosition_2D>();
            if(castPosition->hasLatitudeBeenSet())
                mavlinkItem.lat_int = static_cast<int32_t>(castPosition->getLatitude() * power); bitArray = (bitArray & (~1));
            if(castPosition->hasLongitudeBeenSet())
                mavlinkItem.lon_int = static_cast<int32_t>(castPosition->getLongitude() * power); bitArray = (bitArray & (~2));
        }
        else if(currentTarget.getPosition()->is3D())
        {
            mace::pose::GeodeticPosition_3D* castPosition = currentTarget.getPosition()->positionAs<mace::pose::GeodeticPosition_3D>();
            if(castPosition->hasLatitudeBeenSet())
                mavlinkItem.lat_int = static_cast<int32_t>(castPosition->getLatitude() * power); bitArray = (bitArray & (~1));
            if(castPosition->hasLongitudeBeenSet())
                mavlinkItem.lon_int = static_cast<int32_t>(castPosition->getLongitude() * power); bitArray = (bitArray & (~2));
            if(castPosition->hasAltitudeBeenSet())
                mavlinkItem.alt = static_cast<float>(castPosition->getAltitude());  bitArray = (bitArray & (~4));
        }
    }

    //This handles the packing of the velocity components
    if((currentTarget.getVelocity() != nullptr) && (currentTarget.getVelocity()->isAnyVelocityValid()))
    {
        if(currentTarget.getVelocity()->is2D())
        {
            mace::pose::Cartesian_Velocity2D* castVelocity = currentTarget.getVelocity()->velocityAs<mace::pose::Cartesian_Velocity2D>();
            if(castVelocity->hasXBeenSet())
                mavlinkItem.vx = static_cast<float>(castVelocity->getXVelocity()); bitArray = (bitArray & (~8));
            if(castVelocity->hasYBeenSet())
                mavlinkItem.vy = static_cast<float>(castVelocity->getYVelocity()); bitArray = (bitArray & (~16));
        }
        else if(currentTarget.getVelocity()->is3D())
        {
            mace::pose::Cartesian_Velocity3D* castVelocity = currentTarget.getVelocity()->velocityAs<mace::pose::Cartesian_Velocity3D>();
            if(castVelocity->hasXBeenSet())
                mavlinkItem.vx = static_cast<float>(castVelocity->getXVelocity()); bitArray = (bitArray & (~8));
            if(castVelocity->hasYBeenSet())
                mavlinkItem.vy = static_cast<float>(castVelocity->getYVelocity()); bitArray = (bitArray & (~16));
            if(castVelocity->hasZBeenSet())
                mavlinkItem.vz = static_cast<float>(castVelocity->getZVelocity()); bitArray = (bitArray & (~32));
        }
    }

    if((currentTarget.getYaw() != nullptr) && (currentTarget.getYaw()->isYawDimensionSet()))
    {
        mace::pose::Rotation_2D* castRotation = currentTarget.getYaw();
        mavlinkItem.yaw = static_cast<float>(castRotation->getPhi()); bitArray = (bitArray & (~1024));
    }

    if((currentTarget.getYawRate() != nullptr) && (currentTarget.getYawRate()->isYawDimensionSet()))
    {
        mace::pose::Rotation_2D* castRotation = currentTarget.getYawRate();
        mavlinkItem.yaw_rate = static_cast<float>(castRotation->getPhi()); bitArray = (bitArray & (~2048));
    }
    mavlinkItem.type_mask = bitArray;
}

}// end of namespace MAVLINKVehicleControllers

