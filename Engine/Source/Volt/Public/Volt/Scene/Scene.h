#pragma once

#include "Volt/Asset/AssetTypes.h"

#include "Volt/Particles/ParticleSystem.h"
#include "Volt/Audio/AudioSystem.h"
#include "Volt/Vision/TimelinePlayer.h"

#include "Volt/Scene/WorldEngine/WorldEngine.h"

#include "Volt/Rendering/RendererStructs.h"

#include <AssetSystem/Asset.h>

#include <EventSystem/EventListener.h>
#include <EntitySystem/EntityScene.h>

namespace Volt
{
	class Vision;
	class Entity;

	class Animation;
	class Skeleton;

	class Entity;
	class RenderScene;

	struct SceneSettings
	{
		bool useWorldEngine = false;
	};

	class Scene : public Asset, public EventListener, public std::enable_shared_from_this<Scene>
	{
	public:
		struct Statistics
		{
			uint32_t entityCount = 0;
		};

		Scene();
		Scene(const std::string& name);
		~Scene() override;

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void OnSimulationStart();
		void OnSimulationEnd();

		void Update(float aDeltaTime);
		void FixedUpdate(float aDeltaTime);
		void UpdateEditor(float aDeltaTime);
		void UpdateSimulation(float aDeltaTime);

		void SortScene();

		void MarkEntityAsEdited(const Entity& entity);
		void ClearEditedEntities();

		VT_NODISCARD TQS GetEntityWorldTQS(const Entity& entity) const;

		VT_NODISCARD VT_INLINE entt::registry& GetRegistry() { return m_entityScene.GetRegistry(); }
		VT_NODISCARD VT_INLINE const std::string& GetName() const { return m_name; }
		VT_NODISCARD VT_INLINE const Statistics& GetStatistics() const { return m_statistics; }
		VT_NODISCARD VT_INLINE bool IsPlaying() const { return m_isPlaying; }
		VT_NODISCARD VT_INLINE float GetDeltaTime() const { return m_currentDeltaTime; }

		VT_NODISCARD VT_INLINE SceneSettings& GetSceneSettingsMutable() { return m_sceneSettings; }
		VT_NODISCARD VT_INLINE const SceneSettings& GetSceneSettings() const { return m_sceneSettings; }
		VT_NODISCARD VT_INLINE const WorldEngine& GetWorldEngine() const { return m_worldEngine; }
		VT_NODISCARD VT_INLINE WorldEngine& GetWorldEngineMutable() { return m_worldEngine; }
		VT_NODISCARD VT_INLINE Vision& GetVision() { return *m_visionSystem; }

		VT_NODISCARD VT_INLINE Ref<RenderScene> GetRenderScene() const { return m_renderScene; }

		void SetRenderSize(uint32_t aWidth, uint32_t aHeight);

		Entity CreateEntity(const std::string& tag = "");
		Entity CreateEntityWithID(const EntityID& id, const std::string& tag = "");

		Entity GetEntityFromID(const EntityID id) const;
		Entity GetEntityFromHandle(entt::entity entityHandle) const;

		EntityHelper GetEntityHelperFromEntityID(EntityID entityId) const;

		bool IsRelatedTo(Entity entity, Entity otherEntity);
		void DestroyEntity(Entity entity);
		void ParentEntity(Entity parent, Entity child);
		void UnparentEntity(Entity entity);

		void InvalidateEntityTransform(const EntityID& entityId);
		bool IsEntityValid(EntityID entityId) const;

		template<typename... T>
		Vector<Entity> GetAllEntitiesWith() const;

		template<typename... T>
		Vector<Entity> GetAllEntitiesWith();

		template<typename... T, typename F>
		void ForEachWithComponents(const F& func);

		template<typename EntityType>
		Entity GetSceneEntityFromScriptingEntity(EntityType scriptingEntity);

		Vector<Entity> GetAllEntities() const;
		Vector<Entity> GetAllEditedEntities() const;
		Vector<EntityID> GetAllRemovedEntities() const;

		static Ref<Scene> CreateDefaultScene(const std::string& name, bool createDefaultMesh = true);

		static AssetType GetStaticType() { return AssetTypes::Scene; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

		void CopyTo(Ref<Scene> otherScene);
		void Clear();

	private:
		friend class Entity;
		friend class SceneImporter;
		friend class SceneSerializer;

		void Initialize();

		void IsRecursiveChildOf(Entity mainParent, Entity currentEntity, bool& outChild);
		void ConvertToWorldSpace(Entity entity);
		void ConvertToLocalSpace(Entity entity);

		glm::mat4 GetWorldTransform(Entity entity) const;
		Vector<Entity> FlattenEntityHeirarchy(Entity entity);

		SceneEnvironment m_environment;
		SceneSettings m_sceneSettings;
		Statistics m_statistics;
		WorldEngine m_worldEngine;

		bool m_isPlaying = false;
		float m_timeSinceStart = 0.f;
		float m_currentDeltaTime = 0.f;

		std::string m_name = "New Scene";

		uint32_t m_viewportWidth = 1;
		uint32_t m_viewportHeight = 1;

		EntityScene m_entityScene;

		Ref<Vision> m_visionSystem; // Needs to be of ptr type because of include loop
		Ref<RenderScene> m_renderScene;
	};

	template<typename ...T>
	inline Vector<Entity> Scene::GetAllEntitiesWith()
	{
		Vector<Entity> result{};

		auto view = m_entityScene.GetRegistry().view<T...>();
		for (const auto& ent : view)
		{
			result.emplace_back(GetEntityFromHandle(ent));
		}

		return result;
	}

	template<typename ...T>
	inline Vector<Entity> Scene::GetAllEntitiesWith() const
	{
		Vector<Entity> result{};

		auto view = m_entityScene.GetRegistry().view<T...>();
		for (const auto& ent : view)
		{
			result.emplace_back(GetEntityFromHandle(ent));
		}

		return result;
	}

	template<typename... T, typename F>
	inline void Scene::ForEachWithComponents(const F& func)
	{
		auto view = m_entityScene.GetRegistry().view<T...>();
		view.each(func);
	}

	template<typename EntityType>
	inline Entity Scene::GetSceneEntityFromScriptingEntity(EntityType scriptingEntity)
	{
		return Entity{ scriptingEntity.GetHandle(), this };
	}
}
