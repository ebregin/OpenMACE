#ifndef SPATIAL_LOITER_TIME_H
#define SPATIAL_LOITER_TIME_H

#include <iostream>
#include <iomanip>
#include <sstream>

#include "mace.h"

#include "common/common.h"
#include "common/class_forward.h"

#include "abstract_spatial_action.h"

#include "data_generic_command_item/command_item_type.h"
#include "data/loiter_direction.h"
#include "data_generic_state_item/base_3d_position.h"

#include "data_generic_command_item/abstract_command_item.h"

namespace CommandItem {

MACE_CLASS_FORWARD(SpatialLoiter_Time);

class SpatialLoiter_Time : public AbstractSpatialAction
{

public:
    SpatialLoiter_Time();
    SpatialLoiter_Time(const SpatialLoiter_Time &obj);
    SpatialLoiter_Time(const int &systemOrigin, const int &systemTarget = 0);

public:

    //!
    //! \brief getCommandType returns the type of the object that this command type is.
    //! \return Data::CommandType resolving the type of command this object is.
    //!
    COMMANDITEM getCommandType() const override;

    //!
    //! \brief getDescription
    //! \return string describing the command item. This may be useful for setting up options in a
    //! GUI or somewhere a display needs to interface and decisions have to be made describing what
    //! would happen when issuing such a command.
    //!
    std::string getDescription() const override;

    //!
    //! \brief hasSpatialInfluence returns a boolean reflecting whether or not the commandItem has
    //! a direct influence over a vehicles position. This is useful for determining flight times,
    //! position elements, or rendering objects on a GUI.
    //! \return false if the command does not have an affect over the vehicles position directly.
    //! For example, change speed has no influence over a vehicles position.
    //!
    bool hasSpatialInfluence() const override;

    //!
    //! \brief getClone
    //! \return
    //!
    std::shared_ptr<AbstractCommandItem> getClone() const override;

    /**
     * @brief getClone
     * @param state
     */
    void getClone(std::shared_ptr<AbstractCommandItem> &command) const override;
public:
    void operator = (const SpatialLoiter_Time &rhs)
    {
        AbstractSpatialAction::operator =(rhs);
        this->direction = rhs.direction;
        this->radius = rhs.radius;
        this->duration = rhs.duration;
    }

    bool operator == (const SpatialLoiter_Time &rhs) {
        if(!AbstractSpatialAction::operator ==(rhs))
        {
            return false;
        }
        if(this->direction != rhs.direction)
        {
            return false;
        }
        if(this->radius != rhs.radius)
        {
            return false;
        }
        if(this->duration != rhs.duration)
        {
            return false;
        }
        return true;
    }

    bool operator != (const SpatialLoiter_Time &rhs) {
        return !(*this == rhs);
    }

public:
    //!
    //! \brief printPositionalInfo
    //! \return
    //!
    std::string printSpatialCMDInfo() const override;

public:
    Data::LoiterDirection direction;
    double radius;
    double duration;
};

} //end of namespace MissionItem

#endif // SPATIAL_LOITER_TIME_H
