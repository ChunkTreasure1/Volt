#pragma once

#include <Nexus/Utility/Types.h>
#include <AssetSystem/Asset.h>
#include "Volt/Net/Replicated/ReplicationConditions.h"

namespace Volt
{
	struct NetActorComponent
	{
		eRepCondition condition = eRepCondition::CONTINUOUS;
		bool updateTransformPos = true;
		bool updateTransformRot = true;
		bool updateTransformScale = true;
		uint64_t repId = Nexus::RandRepID();
		uint16_t clientId = 0;

		static void ReflectType(TypeDesc<NetActorComponent>& reflect)
		{
			reflect.SetGUID("{D5A9E480-C9D6-473C-B29E-9FE812320643}"_guid);
			reflect.SetLabel("Net Actor Component");
			reflect.AddMember(&NetActorComponent::condition, "condition", "Condition", "", eRepCondition::CONTINUOUS);
			reflect.AddMember(&NetActorComponent::updateTransformPos, "updateTransformPos", "Update Position", "", true);
			reflect.AddMember(&NetActorComponent::updateTransformRot, "updateTransformRot", "Update Rotation", "", true);
			reflect.AddMember(&NetActorComponent::updateTransformScale, "updateTransformScale", "Update Scale", "", true);
			reflect.AddMember(&NetActorComponent::repId, "repId", "Replication ID", "", Nexus::RandRepID());
			reflect.AddMember(&NetActorComponent::clientId, "clientId", "Client ID", "", 0);
		}

		REGISTER_COMPONENT(NetActorComponent);
	};
}
