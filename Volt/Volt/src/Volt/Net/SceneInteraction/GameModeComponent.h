#pragma once
#include <Wire/Serialization.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct GameModeComponent
	{
		PROPERTY(Name = PlayerPrefab, SpecialType = Asset, AssetType = Prefab) AssetHandle prefabHandle;
		PROPERTY(Name = EnemyPrefab, SpecialType = Asset, AssetType = Prefab) AssetHandle enemy;
		//PROPERTY(Name = Spawn Point) entt::entity spawnPoint = 0;

		CREATE_COMPONENT_GUID("{639786CE-6DE3-4D57-8A00-2C5A0934830A}"_guid);
	}), GameModeComponent);
}
