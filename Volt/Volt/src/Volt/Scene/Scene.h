#pragma once
#include "Volt/Asset/Asset.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Animation/AnimationSystem.h"
#include "Volt/Particles/ParticleSystem.h"
#include "Volt/Audio/AudioSystem.h"
#include "Volt/Vision/TimelinePlayer.h"

#include "Volt/Scripting/Mono/MonoScriptFieldCache.h"

#include <glm/glm.hpp>

#include <entt.hpp>

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
	class Image2D;

	class Entity;
	class RenderScene;

	struct SceneEnvironment
	{
		Ref<Image2D> irradianceMap;
		Ref<Image2D> radianceMap;

		float lod = 0.f;
		float intensity = 1.f;
	};

	struct SceneLayer
	{
		uint32_t id = 0;
		std::string name;

		bool visible = true;
		bool locked = false;
	};

	class Scene : public Asset, public std::enable_shared_from_this<Scene>
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

		inline entt::registry& GetRegistry() { return m_registry; }
		inline const std::string& GetName() const { return m_name; }
		inline const Statistics& GetStatistics() const { return m_statistics; }
		inline const bool IsPlaying() const { return m_isPlaying; }
		inline const float GetDeltaTime() const { return m_currentDeltaTime; }

		void SetTimeScale(const float aTimeScale);

		void InitializeEngineScripts();
		void ShutdownEngineScripts();

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void OnSimulationStart();
		void OnSimulationEnd();

		void Update(float aDeltaTime);
		void FixedUpdate(float aDeltaTime);
		void UpdateEditor(float aDeltaTime);
		void UpdateSimulation(float aDeltaTime);
		void OnEvent(Event& e);

		void SortScene();

		void AddLayer(const std::string& layerName);
		void RemoveLayer(const std::string& layerName);
		void RemoveLayer(uint32_t layerId);
		void MoveToLayer(Entity entity, uint32_t targetLayer);
		void SetLayers(const std::vector<SceneLayer>& sceneLayers); // #TODO_Ivar: We probably don't want to expose this
		void SetActiveLayer(uint32_t layerId);
		bool LayerExists(uint32_t layerId);

		void InvalidateRenderScene();

		inline const uint32_t GetActiveLayer() const { return m_sceneLayers.at(m_activeLayerIndex).id; }
		inline const std::vector<SceneLayer>& GetLayers() const { return m_sceneLayers; }
		inline std::vector<SceneLayer>& GetLayersMutable() { return m_sceneLayers; }

		inline const MonoScriptFieldCache& GetScriptFieldCache() const { return m_monoFieldCache; }
		inline MonoScriptFieldCache& GetScriptFieldCache() { return m_monoFieldCache; }
		const bool IsRelatedTo(Entity entity, Entity otherEntity);

		inline Ref<RenderScene> GetRenderScene() const { return m_renderScene; }

		void SetRenderSize(uint32_t aWidth, uint32_t aHeight);

		Entity CreateEntity(const std::string& tag = "", const entt::entity hintId = entt::null);
		void RemoveEntity(Entity entity);

		void ParentEntity(Entity parent, Entity child);
		void UnparentEntity(Entity entity);

		void InvalidateEntityTransform(entt::entity entity);

		Vision& GetVision() { return *m_visionSystem; }
		TimelinePlayer& GetTimelinePlayer() { return m_timelinePlayer; };

		Entity InstantiateSplitMesh(AssetHandle meshHandle);

		const TQS GetWorldTQS(Entity entity) const;

		const Entity GetEntityWithName(std::string name);

		inline ParticleSystem& GetParticleSystem() { return m_particleSystem; }

		template<typename... T>
		const std::vector<entt::entity> GetAllEntitiesWith() const;

		template<typename... T, typename F>
		void ForEachWithComponents(const F& func);

		const std::vector<entt::entity> GetAllEntities() const;

		static const std::set<AssetHandle> GetDependencyList(const std::filesystem::path& scenePath);
		static bool IsSceneFullyLoaded(const std::filesystem::path& scenePath);
		static void PreloadSceneAssets(const std::filesystem::path& scenePath);
		static Ref<Scene> CreateDefaultScene(const std::string& name, bool createDefaultMesh = true);

		static AssetType GetStaticType() { return AssetType::Scene; }
		AssetType GetType() override { return GetStaticType(); }

		void CopyTo(Ref<Scene> otherScene);
		void Clear();

	private:
		friend class Entity;
		friend class SceneImporter;

		void MoveToLayerRecursive(Entity entity, uint32_t targetLayer);

		void SetupComponentFunctions();

		void IsRecursiveChildOf(Entity mainParent, Entity currentEntity, bool& outChild);
		void ConvertToWorldSpace(Entity entity);
		void ConvertToLocalSpace(Entity entity);

		void RemoveEntityInternal(Entity entity, bool removingParent);

		void AddLayer(const std::string& layerName, uint32_t layerId);

		const glm::mat4 GetWorldTransform(Entity entity) const;
		const std::vector<Entity> FlattenEntityHeirarchy(Entity entity);

		///// Component Functions /////
		void RigidbodyComponent_OnCreate(entt::registry& registry, entt::entity id);
		void CharacterControllerComponent_OnCreate(entt::registry& registry, entt::entity id);
		void BoxColliderComponent_OnCreate(entt::registry& registry, entt::entity id);
		void SphereColliderComponent_OnCreate(entt::registry& registry, entt::entity id);
		void CapsuleColliderComponent_OnCreate(entt::registry& registry, entt::entity id);
		void MeshColliderComponent_OnCreate(entt::registry& registry, entt::entity id);
		void AudioSourceComponent_OnCreate(entt::registry& registry, entt::entity id);
		void AudioListenerComponent_OnCreate(entt::registry& registry, entt::entity id);
		void CameraComponent_OnCreate(entt::registry& registry, entt::entity id);

		void RigidbodyComponent_OnDestroy(entt::registry& registry, entt::entity id);
		void CharacterControllerComponent_OnDestroy(entt::registry& registry, entt::entity id);
		void BoxColliderComponent_OnDestroy(entt::registry& registry, entt::entity id);
		void SphereColliderComponent_OnDestroy(entt::registry& registry, entt::entity id);
		void CapsuleColliderComponent_OnDestroy(entt::registry& registry, entt::entity id);
		void MeshColliderComponent_OnDestroy(entt::registry& registry, entt::entity id);
		//////////////////////////////

		SceneEnvironment m_environment;
		Statistics m_statistics;

		bool m_isPlaying = false;
		float m_timeSinceStart = 0.f;
		float m_currentDeltaTime = 0.f;

		std::string m_name = "New Scene";
		entt::registry m_registry;

		std::vector<SceneLayer> m_sceneLayers;

		mutable std::unordered_map<entt::entity, glm::mat4> m_cachedEntityTransforms;
		mutable std::shared_mutex m_cachedEntityTransformMutex;

		uint32_t m_viewportWidth = 1;
		uint32_t m_viewportHeight = 1;

		uint32_t m_lastLayerId = 1;
		uint32_t m_activeLayerIndex = 0;
		TimelinePlayer m_timelinePlayer;
			
		ParticleSystem m_particleSystem;
		AudioSystem m_audioSystem;
		AnimationSystem m_animationSystem;
		MonoScriptFieldCache m_monoFieldCache;

		Ref<Vision> m_visionSystem; // Needs to be of ptr type because of include loop
		Ref<RenderScene> m_renderScene;
	};

	template<typename ...T>
	inline const std::vector<entt::entity> Scene::GetAllEntitiesWith() const
	{
		std::vector<entt::entity> result{};

		auto view = m_registry.view<T...>();
		for (const auto& ent : view)
		{
			result.emplace_back(ent);
		}

		return result;
	}

	template<typename... T, typename F>
	inline void Scene::ForEachWithComponents(const F& func)
	{
		VT_PROFILE_FUNCTION();

		auto view = m_registry.view<T...>();
		view.each(func);
	}
}
