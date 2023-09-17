#pragma once

#include "Volt/Scene/Reflection/ComponentReflection.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"

namespace Volt
{
	struct GameModeComponent
	{
		AssetHandle prefabHandle;
		AssetHandle enemy;
		//PROPERTY(Name = Spawn Point) entt::entity spawnPoint = 0;

		static void ReflectType(TypeDesc<GameModeComponent>& reflect)
		{
			reflect.SetGUID("{639786CE-6DE3-4D57-8A00-2C5A0934830A}"_guid);
			reflect.SetLabel("Game Mode Component");
			reflect.AddMember(&GameModeComponent::prefabHandle, "prefabHandle", "Prefab", "", Asset::Null());
			reflect.AddMember(&GameModeComponent::enemy, "enemy", "Enemy", "", Asset::Null());
		}

		REGISTER_COMPONENT(GameModeComponent);
	};
}
