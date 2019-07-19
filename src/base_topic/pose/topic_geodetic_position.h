#ifndef TOPIC_GEODETIC_POSITION_H
#define TOPIC_GEODETIC_POSITION_H

#include "data/i_topic_component_data_object.h"

#include "base/pose/geodetic_position_2D.h"
#include "base/pose/geodetic_position_3D.h"

namespace pose {
namespace BaseTopic {

extern const char TopicName_GeodeticPosition[];

extern const MaceCore::TopicComponentStructure Structure_GeodeticPosition;

class Topic_GeodeticPosition : public Data::NamedTopicComponentDataObject<TopicName_GeodeticPosition, &Structure_GeodeticPosition>
{

public:

    Topic_GeodeticPosition();

    Topic_GeodeticPosition(const Topic_GeodeticPosition &copyObj);

    Topic_GeodeticPosition(const mace::pose::Abstract_GeodeticPosition *posObj);

    virtual ~Topic_GeodeticPosition()
    {
        delete m_PositionObject;
    }

public:
    virtual MaceCore::TopicDatagram GenerateDatagram() const;

    virtual void CreateFromDatagram(const MaceCore::TopicDatagram &datagram);

    mace::pose::Abstract_GeodeticPosition* getPositionObj() const;

private:
    mace::pose::Abstract_GeodeticPosition* m_PositionObject;
};

} //end of namespace BaseTopic
} //end of namespace pose

#endif // TOPIC_GEODETIC_POSITION_H