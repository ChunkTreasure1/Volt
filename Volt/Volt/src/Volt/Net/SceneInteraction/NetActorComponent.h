#pragma once

#include <Nexus/Core/Types.h>
#include <Nexus/Utility/Random/Random.h>
#include "Volt/Net/Replicated/ReplicationConditions.h"

namespace Volt
{
	SERIALIZE_COMPONENT((struct NetActorComponent
	{
		PROPERTY(Name = Condition, SpecialType = Enum) eRepCondition condition = eRepCondition::CONTINUOUS;
		PROPERTY(Name = ID) NXS_TYPE_REP_ID repId = Nexus::Random<NXS_TYPE_REP_ID>();
		PROPERTY(Name = cID) uint16_t clientId = 0;
		PROPERTY(Name = Update Transform) bool updateTransform = true;

		CREATE_COMPONENT_GUID("{D5A9E480-C9D6-473C-B29E-9FE812320643}"_guid);
	}), NetActorComponent);
}
