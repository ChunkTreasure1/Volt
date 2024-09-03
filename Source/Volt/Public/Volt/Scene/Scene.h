#pragma once

#include "Volt/Asset/AssetTypes.h"

#include "Volt/Particles/ParticleSystem.h"
#include "Volt/Audio/AudioSystem.h"
#include "Volt/Vision/TimelinePlayer.h"

#include "Volt/Scene/EntityRegistry.h"
#include "Volt/Scene/WorldEngine/WorldEngine.h"

#include "Volt/Rendering/RendererStructs.h"

#include <AssetSystem/Asset.h>

#include <EventSystem/EventListener.h>
#include <EntitySystem/Scripting/ECSBuilder.h>

#include <glm/glm.hpp>

#include <map>
#include <set>

namespace Volt
{
	class ParticleSystem;
	class Vision;
	class AudioSystem;
	class Entity;

	class Animation;
	class Skeleton;
	class AnimatedCharacter;
	class Event;

	class Entity;
	class RenderScene;

	class AppPostFrameUpdateEvent;

	struct SceneSettings
	{
		bool useWorldEngine = false;
	};

	struct SceneLayer
	{
		uint32_t id = 0;
		std::string name;

		bool visible = true;
		bool locked = false;
	};

	class Scene : public Asset, public EventListener, public std::enable_shared_from_this<Scene>
	{
	public:
		struct Statistics
		{
			uint32_t entityCount = 0;
		};

		struct TQS
		{
			glm::vec3 position = { 0.f, 0.f, 0.f };
			glm::quat rotation = { 1.f, 0.f, 0.f, 0.f };
			glm::vec3 scale = { 1.f, 1.f, 1.f };
		};

		Scene();
		Scene(const std::string& name);
		~Scene() override;

		inline entt::registry& GetRegistry() { return m_registry; }
		inline const std::string& GetName() const { return m_name; }
		inline const Statistics& GetStatistics() const { return m_statistics; }
		inline const bool IsPlaying() const { return m_isPlaying; }
		inline const float GetDeltaTime() const { return m_currentDeltaTime; }

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void OnSimulationStart();
		void OnSimulationEnd();

		void Update(float aDeltaTime);
		void FixedUpdate(float aDeltaTime);
		void UpdateEditor(float aDeltaTime);
		void UpdateSimulation(float aDeltaTime);

		void SortScene();

		void AddLayer(const std::string& layerName);
		void RemoveLayer(const std::string& layerName);
		void RemoveLayer(uint32_t layerId);
		void MoveToLayer(Entity entity, uint32_t targetLayer);
		void SetActiveLayer(uint32_t layerId);
		bool LayerExists(uint32_t layerId);

		void MarkEntityAsEdited(const Entity& entity);
		void ClearEditedEntities();
		void InvalidateRenderScene();

		inline const uint32_t GetActiveLayer() const { return m_sceneLayers.at(m_activeLayerIndex).id; }
		inline const Vector<SceneLayer>& GetLayers() const { return m_sceneLayers; }
		inline Vector<SceneLayer>& GetLayersMutable() { return m_sceneLayers; }

		inline SceneSettings& GetSceneSettingsMutable() { return m_sceneSettings; }
		inline const SceneSettings& GetSceneSettings() const { return m_sceneSettings; }

		inline const WorldEngine& GetWorldEngine() const { return m_worldEngine; }
		inline WorldEngine& GetWorldEngineMutable() { return m_worldEngine; }

		inline Ref<RenderScene> GetRenderScene() const { return m_renderScene; }

		void SetRenderSize(uint32_t aWidth, uint32_t aHeight);

		Entity CreateEntity(const std::string& tag = "");
		Entity CreateEntityWithUUID(const EntityID& uuid, const std::string& tag = "");

		Entity GetEntityFromUUID(const EntityID uuid) const;
		entt::entity GetHandleFromUUID(const EntityID uuid) const;

		const bool IsRelatedTo(Entity entity, Entity otherEntity);
		void RemoveEntity(Entity entity);
		void ParentEntity(Entity parent, Entity child);
		void UnparentEntity(Entity entity);

		void InvalidateEntityTransform(const EntityID& entityUUID);

		Vision& GetVision() { return *m_visionSystem; }
		TimelinePlayer& GetTimelinePlayer() { return m_timelinePlayer; };

		const TQS GetWorldTQS(Entity entity) const;
		const Entity GetEntityWithName(std::string name);
		const bool IsEntityValid(EntityID entityId) const;

		inline ParticleSystem& GetParticleSystem() { return m_particleSystem; }

		template<typename... T>
		const Vector<Entity> GetAllEntitiesWith() const;

		template<typename... T>
		Vector<Entity> GetAllEntitiesWith();

		template<typename... T, typename F>
		void ForEachWithComponents(const F& func);

		const Vector<Entity> GetAllEntities() const;
		const Vector<Entity> GetAllEditedEntities() const;
		const Vector<EntityID> GetAllRemovedEntities() const;

		static const std::set<AssetHandle> GetDependencyList(const std::filesystem::path& scenePath);
		static bool IsSceneFullyLoaded(const std::filesystem::path& scenePath);
		static void PreloadSceneAssets(const std::filesystem::path& scenePath);
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
		void ComponentOnStart();
		void ComponentOnStop();

		void MoveToLayerRecursive(Entity entity, uint32_t targetLayer);

		void IsRecursiveChildOf(Entity mainParent, Entity currentEntity, bool& outChild);
		void ConvertToWorldSpace(Entity entity);
		void ConvertToLocalSpace(Entity entity);

		void RemoveEntityInternal(Entity entity, bool removingParent);

		void AddLayer(const std::string& layerName, uint32_t layerId);

		const glm::mat4 GetWorldTransform(Entity entity) const;
		const Vector<Entity> FlattenEntityHeirarchy(Entity entity);

		SceneEnvironment m_environment;
		SceneSettings m_sceneSettings;
		Statistics m_statistics;
		WorldEngine m_worldEngine;

		bool m_isPlaying = false;
		float m_timeSinceStart = 0.f;
		float m_currentDeltaTime = 0.f;

		std::string m_name = "New Scene";
		entt::registry m_registry;

		Vector<SceneLayer> m_sceneLayers;

		std::mutex m_removeEntityMutex;
		Vector<EntityID> m_entityRemoveQueue;

		mutable std::unordered_map<EntityID, glm::mat4> m_cachedEntityTransforms;
		mutable std::shared_mutex m_cachedEntityTransformMutex;

		EntityRegistry m_entityRegistry{};

		uint32_t m_viewportWidth = 1;
		uint32_t m_viewportHeight = 1;

		uint32_t m_lastLayerId = 1;
		uint32_t m_activeLayerIndex = 0;
		TimelinePlayer m_timelinePlayer;

		ParticleSystem m_particleSystem;
		AudioSystem m_audioSystem;

		Ref<Vision> m_visionSystem; // Needs to be of ptr type because of include loop
		Ref<RenderScene> m_renderScene;

		ECSBuilder m_ecsBuilder;
	};

	template<typename ...T>
	inline Vector<Entity> Scene::GetAllEntitiesWith()
	{
		Vector<Entity> result{};

		auto view = m_registry.view<T...>();
		for (const auto& ent : view)
		{
			result.emplace_back(GetEntityFromUUID(m_entityRegistry.GetUUIDFromHandle(ent)));
		}

		return result;
	}

	template<typename ...T>
	inline const Vector<Entity> Scene::GetAllEntitiesWith() const
	{
		Vector<Entity> result{};

		auto view = m_registry.view<T...>();
		for (const auto& ent : view)
		{
			result.emplace_back(GetEntityFromUUID(m_entityRegistry.GetUUIDFromHandle(ent)));
		}

		return result;
	}

	template<typename... T, typename F>
	inline void Scene::ForEachWithComponents(const F& func)
	{
		auto view = m_registry.view<T...>();
		view.each(func);
	}
}
