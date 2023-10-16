#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Scene/Entity.h"

#include <entt.hpp>

namespace Volt
{
	class Scene;

	class Prefab : public Asset
	{
	public:
		struct PrefabReferenceData
		{
			AssetHandle prefabAsset;
			entt::entity prefabReferenceEntity;
		};

		Prefab() = default;
		Prefab(Entity srcRootEntity);
		Prefab(Ref<Scene> prefabScene, entt::entity rootEntityId, uint32_t version);

		~Prefab() override = default;

		Entity Instantiate(Weak<Scene> targetScene);
		const bool UpdateEntityInPrefab(Entity srcEntity);
		void UpdateEntityInScene(Entity sceneEntity);

		void CopyPrefabEntity(Entity dstEntity, entt::entity srcPrefabEntityId, const EntityCopyFlags copyFlags = EntityCopyFlags::SkipRelationships) const;

		[[nodiscard]] inline const bool IsPrefabValid() { return m_prefabScene != nullptr && m_rootEntityId != entt::null; }
		[[nodiscard]] const bool IsEntityValidInPrefab(Entity entity) const;
		[[nodiscard]] const bool IsEntityValidInPrefab(entt::entity prefabEntityId) const;
		[[nodiscard]] const bool IsEntityRoot(Entity entity) const;
		[[nodiscard]] const bool IsReference(Entity entity) const;

		[[nodiscard]] const PrefabReferenceData& GetReferenceData(Entity entity) const;

		static AssetType GetStaticType() { return AssetType::Prefab; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		friend class PrefabImporter;

		void InitializeComponents(Entity entity);

		void CreatePrefab(Entity srcRootEntity);
		void AddEntityToPrefabRecursive(Entity entity, Entity parentPrefabEntity);
		void ValidatePrefabUpdate(Entity srcEntity);

		const bool UpdateEntityInPrefabInternal(Entity srcEntity, entt::entity rootSceneId, entt::entity forcedPrefabEntity);
		void UpdateEntityInSceneInternal(Entity sceneEntity, entt::entity forcedPrefabEntity);

		Entity InstantiateEntity(Weak<Scene> targetScene, Entity prefabEntity);
		const std::vector<Entity> FlattenEntityHeirarchy(Entity entity);

		Ref<Scene> m_prefabScene;
		std::unordered_map<entt::entity, PrefabReferenceData> m_prefabReferencesMap; // Maps this prefabs entity to an entity in another prefab

		entt::entity m_rootEntityId = entt::null;
		uint32_t m_version = 0;
	};
}
