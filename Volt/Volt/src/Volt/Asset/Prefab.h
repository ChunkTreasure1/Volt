#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Scene/Entity.h"

namespace Volt
{
	class Scene;

	class Prefab : public Asset
	{
	public:
		struct PrefabReferenceData
		{
			AssetHandle prefabAsset;
			EntityID prefabReferenceEntity;
		};

		Prefab() = default;
		Prefab(Entity srcRootEntity);
		Prefab(Ref<Scene> prefabScene, EntityID rootEntityId, uint32_t version);

		~Prefab() override = default;

		Entity Instantiate(Weak<Scene> targetScene);
		const bool UpdateEntityInPrefab(Entity srcEntity);
		void UpdateEntityInScene(Entity sceneEntity);

		void CopyPrefabEntity(Entity dstEntity, EntityID srcPrefabEntityId, const EntityCopyFlags copyFlags = EntityCopyFlags::SkipRelationships) const;

		[[nodiscard]] inline const bool IsPrefabValid() { return m_prefabScene != nullptr && m_rootEntityId != Entity::NullID(); }
		[[nodiscard]] const bool IsEntityValidInPrefab(Entity entity) const;
		[[nodiscard]] const bool IsEntityValidInPrefab(EntityID prefabEntityId) const;
		[[nodiscard]] const bool IsEntityRoot(Entity entity) const;
		[[nodiscard]] const bool IsReference(Entity entity) const;

		[[nodiscard]] const PrefabReferenceData& GetReferenceData(Entity entity) const;

		static AssetType GetStaticType() { return AssetType::Prefab; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return m_version; }

	private:
		friend class PrefabImporter;
		friend class PrefabSerializer;

		[[nodiscard]] const Entity GetRootEntity() const;

		void InitializeComponents(Entity entity);

		void CreatePrefab(Entity srcRootEntity);
		void AddEntityToPrefabRecursive(Entity entity, Entity parentPrefabEntity);
		void ValidatePrefabUpdate(Entity srcEntity);
		void UpdatePrefabVersion(Entity entity, uint32_t targetVersion);

		const bool UpdateEntityInPrefabInternal(Entity srcEntity, EntityID rootSceneId, EntityID forcedPrefabEntity);
		void UpdateEntityInSceneInternal(Entity sceneEntity, EntityID forcedPrefabEntity);

		Entity InstantiateEntity(Weak<Scene> targetScene, Entity prefabEntity);
		const Vector<Entity> FlattenEntityHeirarchy(Entity entity);

		Ref<Scene> m_prefabScene;
		std::unordered_map<EntityID, PrefabReferenceData> m_prefabReferencesMap; // Maps this prefabs entity to an entity in another prefab

		EntityID m_rootEntityId = Entity::NullID();
		uint32_t m_version = 0;
	};
}
