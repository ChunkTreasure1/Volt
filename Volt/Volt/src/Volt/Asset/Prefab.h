#pragma once

#include "Volt/Asset/Asset.h"

#include <Wire/Registry.h>

namespace Volt
{
	class Prefab : public Asset
	{
	public:
		Prefab() = default;
		Prefab(Wire::Registry& aParentRegistry, Wire::EntityId topEntity);
		~Prefab() override = default;

		Wire::EntityId Instantiate(Wire::Registry& aTargetRegistry);

		static AssetType GetStaticType() { return AssetType::Prefab; }
		AssetType GetType() override { return GetStaticType(); };

		static void OverridePrefabInRegistry(Wire::Registry& aTargetRegistry, Wire::EntityId aRootEntity, AssetHandle aPrefabId);
		static void OverridePrefabsInRegistry(Wire::Registry& aTargetRegistry, AssetHandle aPrefabId);
		static void OverridePrefabAsset(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity, AssetHandle aPrefabId);

		static bool IsParentInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId);
		static uint32_t GetPrefabVersion(AssetHandle aPrefabId);

	private:
		friend class PrefabImporter;
		
		static void OverrideEntityInPrefab(Wire::Registry& aTargetRegistry, Wire::EntityId aEntity, Ref<Prefab> aPrefab);

		Wire::EntityId InstantiateEntity(Wire::Registry& aTargetRegistry, Wire::EntityId prefabEntity, Wire::EntityId parentEntity);
		void AddToPrefab(Wire::Registry& aParentRegistry, Wire::EntityId entity, uint32_t& count);
		void OverrideEntity(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity);

		Wire::Registry myRegistry;
		uint32_t myVersion = 0;
	};
}