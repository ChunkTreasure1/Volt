#pragma once

#include "EntitySystem/ComponentRegistry.h"
#include "EntitySystem/Scripting/ECSAccessBuilder.h"
#include "EntitySystem/EntityID.h"

namespace Volt
{
	struct VTES_API CommonComponent
	{
		uint32_t layerId = 0;
		float randomValue = 0.f;
		float timeSinceCreation = 0.f;

		uint64_t timeCreatedID = 0;

		static void ReflectType(TypeDesc<CommonComponent>& reflect)
		{
			reflect.SetGUID("{A6789316-2D82-46FC-8138-B7BCBB9EA5B8}"_guid);
			reflect.SetLabel("Common Component");
			reflect.SetHidden();
			reflect.AddMember(&CommonComponent::layerId, "layerid", "Layer ID", "", 0u);
			reflect.AddMember(&CommonComponent::timeCreatedID, "timecreatedid", "Time Created", "", 0u);
			reflect.SetOnStartCallback(&CommonComponent::OnStart);
		}

		REGISTER_COMPONENT(CommonComponent);

	private:
		using CommonEntity = ECS::Access
			::Write<CommonComponent>
			::As<ECS::Type::Entity>;

		static void OnStart(CommonEntity entity);
	};
}
