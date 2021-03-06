#ifndef VELOCITY_INTERFACE_H
#define VELOCITY_INTERFACE_H

#include <Eigen/Core>
#include <type_traits>

#include "abstract_velocity.h"

namespace mace{
namespace pose{

template<const CoordinateSystemTypes coordType, typename CFDATA, class DATA>
class VelocityInterface: public Velocity
{
public:
    VelocityInterface():
        Velocity(), explicitFrame()
    {

    }

    VelocityInterface(const CFDATA &frame):
        Velocity(), explicitFrame(frame)
    {
        explicitType = coordType;
    }

    VelocityInterface(const VelocityInterface &copy):
        Velocity(copy)
    {
        explicitType = copy.explicitType;
        explicitFrame = copy.explicitFrame;
        data = copy.data;
    }

    Eigen::VectorXd getDataVector() const override
    {
        return data;
    }

    void updateDataVector(const Eigen::VectorXd &vecObj) override
    {
        data = vecObj.head(dimension);
    }

public:
    Velocity* getVelocityClone() const override
    {
        return (new VelocityInterface<coordType, CFDATA, DATA>(*this));
    }

    void getVelocityClone(Velocity** state) const override
    {
        *state = new VelocityInterface<coordType, CFDATA, DATA>(*this);
    }

    CoordinateSystemTypes getCoordinateSystemType() const override
    {
        return explicitType;
    }


public:
    CoordinateFrameTypes getExplicitCoordinateFrame() const
    {
        return getCoordinateFrame(explicitFrame);
    }

    void setExplicitCoordinateFrame(const CFDATA &frame)
    {
        explicitFrame = frame;
    }

public:
    bool operator == (const VelocityInterface &rhs) const
    {
        if(!Velocity::operator ==(rhs))
            return false;

        if(explicitType != rhs.explicitType){
            return false;
        }
        if(explicitFrame != rhs.explicitFrame){
            return false;
        }
        if(data != rhs.data){
            return false;
        }
        return true;
    }

    bool operator !=(const Velocity &rhs) const
    {
        return !(*this == rhs);
    }

private:
    CoordinateSystemTypes explicitType = CoordinateSystemTypes::UNKNOWN;
    CFDATA explicitFrame;

public:
    DATA data;
};


} // end of namespace pose
} // end of namespace mace

#endif // VELOCITY_INTERFACE_H
