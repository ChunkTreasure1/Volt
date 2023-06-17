#pragma once

#include <Nexus/Utility/Types.h>
#include "Volt/Asset/Asset.h"
#include "Volt/Net/Replicated/ReplicationConditions.h"

namespace Volt
{
	SERIALIZE_COMPONENT((struct NetActorComponent
	{
		PROPERTY(Name = Condition, SpecialType = Enum) eRepCondition condition = eRepCondition::CONTINUOUS;
		PROPERTY(Name = Update Position) int updateTransformPos = 1;
		PROPERTY(Name = Update Rotation) int updateTransformRot = 1;
		PROPERTY(Name = Update Scale) int updateTransformScale = 1;
		PROPERTY(Name = RepId) uint64_t repId = Nexus::RandRepID();
		PROPERTY(Name = cID) uint16_t clientId = 0;

		CREATE_COMPONENT_GUID("{D5A9E480-C9D6-473C-B29E-9FE812320643}"_guid);
	}), NetActorComponent);
}
