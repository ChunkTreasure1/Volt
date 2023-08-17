#pragma once
#include "Volt/Asset/Asset.h"

#include "Volt/Animation/AnimationSystem.h"
#include "Volt/Particles/ParticleSystem.h"
#include "Volt/Audio/AudioSystem.h"
#include "Volt/Vision/TimelinePlayer.h"

#include "Volt/Scripting/Mono/MonoScriptFieldCache.h"

#include <Wire/Wire.h>
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
	class Image2D;

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

	class Scene : public Asset
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

		inline Wire::Registry& GetRegistry() { return myRegistry; }
		inline const std::string& GetName() const { return myName; }
		inline const Statistics& GetStatistics() const { return myStatistics; }
		inline const bool IsPlaying() const { return myIsPlaying; }
		inline const float GetDeltaTime() const { return myCurrentDeltaTime; }

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
		void SetActiveLayer(uint32_t layerId);
		bool LayerExists(uint32_t layerId);

		inline const uint32_t GetActiveLayer() const { return mySceneLayers.at(myActiveLayerIndex).id; }
		inline const std::vector<SceneLayer>& GetLayers() const { return mySceneLayers; }
		inline std::vector<SceneLayer>& GetLayersMutable() { return mySceneLayers; }

		inline const MonoScriptFieldCache& GetScriptFieldCache() const { return myMonoFieldCache; }
		inline MonoScriptFieldCache& GetScriptFieldCache() { return myMonoFieldCache; }

		inline Ref<RenderScene> GetRenderScene() const { return m_renderScene; }

		void SetRenderSize(uint32_t aWidth, uint32_t aHeight);

		Entity CreateEntity(const std::string& tag = "");
		void RemoveEntity(Entity entity);
		void RemoveEntity(Entity entity, float aTimeToDestroy);

		void ParentEntity(Entity parent, Entity child);
		void UnparentEntity(Entity entity);

		glm::mat4 GetWorldSpaceTransform(Entity entity);
		TQS GetWorldSpaceTRS(Entity entity);

		void InvalidateEntityTransform(Wire::EntityId entity);

		Vision& GetVision() { return *myVisionSystem; }
		TimelinePlayer& GetTimelinePlayer() { return myTimelinePlayer; };

		Entity InstantiateSplitMesh(AssetHandle meshHandle);

		glm::vec3 GetWorldForward(Entity entity);
		glm::vec3 GetWorldRight(Entity entity);
		glm::vec3 GetWorldUp(Entity entity);

		template<typename T>
		const std::vector<Wire::EntityId> GetAllEntitiesWith() const;

		const Entity GetEntityWithName(std::string name);

		inline ParticleSystem& GetParticleSystem() { return myParticleSystem; }

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

		void SetupComponentCreationFunctions();
		void SetupComponentDeletionFunctions();

		void IsRecursiveChildOf(Entity mainParent, Entity currentEntity, bool& outChild);
		void ConvertToWorldSpace(Entity entity);
		void ConvertToLocalSpace(Entity entity);

		void AddLayer(const std::string& layerName, uint32_t layerId);

		SceneEnvironment myEnvironment;
		Statistics myStatistics;

		bool myIsPlaying = false;
		float myTimeSinceStart = 0.f;
		float myCurrentDeltaTime = 0.f;

		std::string myName = "New Scene";
		Wire::Registry myRegistry;

		std::map<Wire::EntityId, bool> myEntityTimesToDestroyRemoved;
		std::map<Wire::EntityId, float> myEntityTimesToDestroy;

		std::vector<SceneLayer> mySceneLayers;
		std::unordered_map<Wire::EntityId, glm::mat4> myCachedEntityTransforms;
		std::shared_mutex myCachedEntityTransformMutex;

		uint32_t myWidth = 1;
		uint32_t myHeight = 1;

		uint32_t myLastLayerId = 1;
		uint32_t myActiveLayerIndex = 0;
		TimelinePlayer myTimelinePlayer;

		ParticleSystem myParticleSystem;
		AudioSystem myAudioSystem;
		AnimationSystem myAnimationSystem;
		MonoScriptFieldCache myMonoFieldCache;

		Ref<Vision> myVisionSystem; // Needs to be of ptr type because of include loop
		Ref<RenderScene> m_renderScene;
	};

	template<typename T>
	inline const std::vector<Wire::EntityId> Scene::GetAllEntitiesWith() const
	{
		return myRegistry.GetComponentView<T>();
	}
}
