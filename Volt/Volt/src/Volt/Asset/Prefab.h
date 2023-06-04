#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Scripting/Mono/MonoScriptFieldCache.h"

#include <Wire/Registry.h>

namespace Volt
{
	class Scene;
	//class Prefab : public Asset
	//{
	//public:
	//	Prefab() = default;
	//	Prefab(Wire::Registry& aParentRegistry, Wire::EntityId topEntity);
	//	~Prefab() override = default;

	//	Wire::EntityId Instantiate(Scene* targetScene, Wire::EntityId specifiedTargetId = 0);
	//	Wire::EntityId GetRootId();

	//	static AssetType GetStaticType() { return AssetType::Prefab; }
	//	AssetType GetType() override { return GetStaticType(); };

	//	static void OverridePrefabInRegistry(Wire::Registry& aTargetRegistry, Wire::EntityId aRootEntity, AssetHandle aPrefabId);
	//	static void OverridePrefabsInRegistry(Wire::Registry& aTargetRegistry, AssetHandle aPrefabId);
	//	static void OverridePrefabAsset(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity, AssetHandle aPrefabId);

	//	static bool IsRootInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId);
	//	static bool IsValidInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId);
	//	static uint32_t GetPrefabVersion(AssetHandle aPrefabId);

	//	static AssetHandle GetCorrectAssethandle(Wire::EntityId aRootId, AssetHandle aPrefabId);

	//	const Wire::Registry& GetRegistry() { return myRegistry; }

	//private:
	//	friend class PrefabImporter;

	//	static void OverrideEntityInPrefab(Wire::Registry& aTargetRegistry, Wire::EntityId aEntity, Ref<Prefab> aPrefab);

	//	Wire::EntityId InstantiateEntity(Scene* targetScene, Wire::EntityId prefabEntity, Wire::EntityId parentEntity, Wire::EntityId specifiedTargetId = 0);
	//	Wire::EntityId AddToPrefab(Wire::Registry& aParentRegistry, Wire::EntityId entity, uint32_t& count);
	//	void OverrideEntity(Wire::Registry& aSrcRegistry, Wire::EntityId aSrcEntity);

	//	std::vector<Wire::EntityId> myKeepEntitiesQueue;

	//	Wire::Registry myRegistry;
	//	uint32_t myVersion = 0;
	//};

	class Prefab : public Asset
	{
	public:
		Prefab() = default;
		Prefab(Scene* scene, Wire::EntityId rootEntity);
		~Prefab() override = default;

		Wire::EntityId GetRootId() { return myRootId; };
		Wire::EntityId Instantiate(Scene* targetScene, Wire::EntityId aTargetEntity = 0);

		inline const MonoScriptFieldCache& GetScriptFieldCache() const { return myScriptFieldCache; }
		inline MonoScriptFieldCache& GetScriptFieldCache() { return myScriptFieldCache; }

		static void OverridePrefabAsset(Scene* scene, Wire::EntityId aSrcEntity, AssetHandle aPrefabId);
		static Wire::EntityId UpdateEntity(Scene* targetScene, Wire::EntityId aTargetEntity, AssetHandle prefabHandle);
		static bool IsRootInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId);
		static bool IsValidInPrefab(Wire::EntityId aEntityId, AssetHandle aPrefabId);
		static uint32_t GetPrefabVersion(AssetHandle aPrefabId);

		static void PreloadAllPrefabs();

		static AssetType GetStaticType() { return AssetType::Prefab; }
		AssetType GetType() override { return GetStaticType(); };

		Wire::Registry& GetRegistry() { return myRegistry; }

	private:
		friend class PrefabImporter;
		bool CreatePrefab(Scene* scene, Wire::EntityId rootEntity);
		Wire::EntityId RecursiveAddToPrefab(Scene* scene, Wire::EntityId aSrcEntity);
		void UpdateComponents(Scene* targetScene, Wire::EntityId aTargetEntity);

		void CorrectEntityReferences(Scene* scene, Wire::EntityId targetEntity);
		void CorrectEntityReferencesRecursive(Scene* scene, Wire::EntityId targetEntity, Wire::EntityId startEntity);

		Wire::EntityId FindCorrespondingEntity(Wire::Registry& registry, Wire::EntityId startEntity, Wire::EntityId wantedPrefabEntity);

		uint32_t myVersion = 0;
		Wire::EntityId myRootId = Wire::NullID;

		Wire::Registry myRegistry;
		MonoScriptFieldCache myScriptFieldCache;
	};
}
