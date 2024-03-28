#pragma once

#include "Volt/Scene/Reflection/ComponentReflection.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"

namespace Volt
{
	struct GameModeComponent
	{
		AssetHandle prefabHandle;
		AssetHandle enemy;

		static void ReflectType(TypeDesc<GameModeComponent>& reflect)
		{
			reflect.SetGUID("{639786CE-6DE3-4D57-8A00-2C5A0934830A}"_guid);
			reflect.SetLabel("Game Mode Component");
			reflect.AddMember(&GameModeComponent::prefabHandle, "prefabHandle", "Player", "", Asset::Null(), AssetType::Prefab);
			reflect.AddMember(&GameModeComponent::enemy, "enemy", "Enemy", "", Asset::Null(), AssetType::Prefab);
		}

		REGISTER_COMPONENT(GameModeComponent);
	};
}
