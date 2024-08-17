#pragma once

#include <EntitySystem/ComponentRegistry.h>

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
			reflect.AddMember(&GameModeComponent::prefabHandle, "prefabHandle", "Player", "", Asset::Null(), AssetTypes::Prefab);
			reflect.AddMember(&GameModeComponent::enemy, "enemy", "Enemy", "", Asset::Null(), AssetTypes::Prefab);
		}

		REGISTER_COMPONENT(GameModeComponent);
	};
}
