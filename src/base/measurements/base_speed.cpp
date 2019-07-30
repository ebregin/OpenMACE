#include "base_speed.h"

namespace mace {
namespace measurements {

Speed::Speed()
{

}

Speed::Speed(const Speed &copy)
{
    this->speedType = copy.speedType;
    this->measurement = copy.measurement;
}

void Speed::updateSpeedMeasurement(const double &value)
{
    this->measurement = value;
}

void Speed::updateSpeedType(const SpeedTypes &type)
{
    this->speedType = type;
}

double Speed::getSpeedMeasurement() const
{
    return this->measurement;
}

Speed::SpeedTypes Speed::getSpeedType() const
{
    return this->speedType;
}

} //end of namespace measurement
} //end of namespace mace
