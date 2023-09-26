#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Scene/Entity.h"

#include <entt.hpp>

namespace Volt
{

	//class Prefab : public Asset
	//{
	//public:
	//	Prefab() = default;
	//	Prefab(Scene* scene, entt::entity rootEntity);
	//	~Prefab() override = default;

	//	entt::entity GetRootId() { return myRootId; };
	//	entt::entity Instantiate(Scene* targetScene, entt::entity aTargetEntity = entt::null);

	//	inline const MonoScriptFieldCache& GetScriptFieldCache() const { return myScriptFieldCache; }
	//	inline MonoScriptFieldCache& GetScriptFieldCache() { return myScriptFieldCache; }

	//	static void OverridePrefabAsset(Scene* scene, entt::entity aSrcEntity, AssetHandle aPrefabId);
	//	static entt::entity UpdateEntity(Scene* targetScene, entt::entity aTargetEntity, AssetHandle prefabHandle);
	//	static bool IsRootInPrefab(entt::entity aEntityId, AssetHandle aPrefabId);
	//	static bool IsValidInPrefab(entt::entity aEntityId, AssetHandle aPrefabId);
	//	static uint32_t GetPrefabVersion(AssetHandle aPrefabId);

	//	static void PreloadAllPrefabs();

	//	static AssetType GetStaticType() { return AssetType::Prefab; }
	//	AssetType GetType() override { return GetStaticType(); };

	//	entt::registry& GetRegistry() { return myRegistry; }

	//private:
	//	friend class PrefabImporter;
	//	bool CreatePrefab(Scene* scene, entt::entity rootEntity);
	//	entt::entity RecursiveAddToPrefab(Scene* scene, entt::entity aSrcEntity);
	//	void UpdateComponents(Scene* targetScene, entt::entity aTargetEntity);

	//	void CorrectEntityReferences(Scene* scene, entt::entity targetEntity);
	//	void CorrectEntityReferencesRecursive(Scene* scene, entt::entity targetEntity, entt::entity startEntity);

	//	entt::entity FindCorrespondingEntity(Scene* targetScene, entt::entity startEntity, entt::entity wantedPrefabEntity);

	//	uint32_t myVersion = 0;
	//	entt::entity myRootId = entt::null;

	//	entt::registry myRegistry;
	//	MonoScriptFieldCache myScriptFieldCache;
	//};

	class Scene;

	class Prefab : public Asset
	{
	public:
		Prefab() = default;
		Prefab(Entity srcRootEntity);
		~Prefab() override = default;

		Entity Instantiate(Weak<Scene> targetScene);
		const bool UpdateEntityInPrefab(Entity srcEntity);

		[[nodiscard]] inline const bool IsPrefabValid() { return m_prefabScene != nullptr && m_rootEntityId != entt::null; }
		[[nodiscard]] const bool IsEntityValidInPrefab(Entity entity) const;
		[[nodiscard]] const bool IsEntityRoot(Entity entity) const;

		static AssetType GetStaticType() { return AssetType::Prefab; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		friend class PrefabImporter;

		void InitializeComponents(Entity entity);

		void CreatePrefab(Entity srcRootEntity);
		void AddEntityToPrefabRecursive(Entity entity, Entity parentPrefabEntity);

		Ref<Scene> m_prefabScene;

		entt::entity m_rootEntityId = entt::null;
		uint32_t m_version = 0;
	};
}
