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
		Prefab() = default;
		Prefab(Entity srcRootEntity);
		~Prefab() override = default;

		Entity Instantiate(Weak<Scene> targetScene);
		const bool UpdateEntityInPrefab(Entity srcEntity);

		void UpdateEntityInScene(Entity sceneEntity);

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
		void ValidatePrefabUpdate(Entity srcEntity);
		const bool UpdateEntityInPrefabInternal(Entity srcEntity, entt::entity rootSceneId);

		Entity InstantiateEntity(Weak<Scene> targetScene, Entity prefabEntity);
		const std::vector<Entity> FlattenEntityHeirarchy(Entity entity);

		Ref<Scene> m_prefabScene;

		entt::entity m_rootEntityId = entt::null;
		uint32_t m_version = 0;
	};
}
