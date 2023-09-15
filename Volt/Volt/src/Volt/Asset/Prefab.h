#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Scripting/Mono/MonoScriptFieldCache.h"

#include <entt.hpp>

namespace Volt
{
	class Scene;

	class Prefab : public Asset
	{
	public:
		Prefab() = default;
		Prefab(Scene* scene, entt::entity rootEntity);
		~Prefab() override = default;

		entt::entity GetRootId() { return myRootId; };
		entt::entity Instantiate(Scene* targetScene, entt::entity aTargetEntity = entt::null);

		inline const MonoScriptFieldCache& GetScriptFieldCache() const { return myScriptFieldCache; }
		inline MonoScriptFieldCache& GetScriptFieldCache() { return myScriptFieldCache; }

		static void OverridePrefabAsset(Scene* scene, entt::entity aSrcEntity, AssetHandle aPrefabId);
		static entt::entity UpdateEntity(Scene* targetScene, entt::entity aTargetEntity, AssetHandle prefabHandle);
		static bool IsRootInPrefab(entt::entity aEntityId, AssetHandle aPrefabId);
		static bool IsValidInPrefab(entt::entity aEntityId, AssetHandle aPrefabId);
		static uint32_t GetPrefabVersion(AssetHandle aPrefabId);

		static void PreloadAllPrefabs();

		static AssetType GetStaticType() { return AssetType::Prefab; }
		AssetType GetType() override { return GetStaticType(); };

		entt::registry& GetRegistry() { return myRegistry; }

	private:
		friend class PrefabImporter;
		bool CreatePrefab(Scene* scene, entt::entity rootEntity);
		entt::entity RecursiveAddToPrefab(Scene* scene, entt::entity aSrcEntity);
		void UpdateComponents(Scene* targetScene, entt::entity aTargetEntity);

		void CorrectEntityReferences(Scene* scene, entt::entity targetEntity);
		void CorrectEntityReferencesRecursive(Scene* scene, entt::entity targetEntity, entt::entity startEntity);

		entt::entity FindCorrespondingEntity(Scene* targetScene, entt::entity startEntity, entt::entity wantedPrefabEntity);

		uint32_t myVersion = 0;
		entt::entity myRootId = entt::null;

		entt::registry myRegistry;
		MonoScriptFieldCache myScriptFieldCache;
	};
}
