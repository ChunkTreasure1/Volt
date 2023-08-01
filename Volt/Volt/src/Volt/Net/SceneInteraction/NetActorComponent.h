#pragma once

#include <Nexus/Core/Types/Types.h>
#include "Volt/Net/Replicated/ReplicationConditions.h"

namespace Volt
{
	SERIALIZE_COMPONENT((struct NetActorComponent
	{
		PROPERTY(Name = Condition, SpecialType = Enum) eRepCondition condition = eRepCondition::CONTINUOUS;
		PROPERTY(Name = ID) uint16_t repId = Nexus::TYPE::RandRepID();
		PROPERTY(Name = cID) uint16_t clientId = 0;
		PROPERTY(Name = Update Transform) bool updateTransform = true;

		CREATE_COMPONENT_GUID("{D5A9E480-C9D6-473C-B29E-9FE812320643}"_guid);
	}), NetActorComponent);
}
