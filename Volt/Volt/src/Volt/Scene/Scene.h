#pragma once
#include "Volt/Asset/Asset.h"

#include <Wire/Wire.h>
#include <gem/gem.h>

#include <map>
#include <set>

namespace Volt
{
	class ParticleSystem;
	class Entity;
	class TextureCube;

	class Animation;
	class Skeleton;
	class AnimatedCharacter;
	class Event;
	class Image2D;

	class PhysicsSystem;

	struct SceneEnvironment
	{
		Ref<Image2D> irradianceMap;
		Ref<Image2D> radianceMap;

		float lod = 0.f;
		float intensity = 1.f;
	};

	class Scene : public Asset
	{
	public:
		struct Statistics
		{
			uint32_t entityCount = 0;
		};

		struct TRS
		{
			gem::vec3 position = { 0.f, 0.f, 0.f };
			gem::vec3 rotation = { 0.f, 0.f, 0.f };
			gem::vec3 scale = { 1.f, 1.f, 1.f };
		};

		Scene();
		Scene(const std::string& name);

		inline Wire::Registry& GetRegistry() { return myRegistry; }
		inline const std::string& GetName() const { return myName; }
		inline const Statistics& GetStatistics() const { return myStatistics; }
		inline const bool IsPlaying() const { return myIsPlaying; }

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void OnSimulationStart();
		void OnSimulationEnd();

		void Update(float aDeltaTime);
		void UpdateEditor(float aDeltaTime);
		void UpdateSimulation(float aDeltaTime);
		void OnEvent(Event& e);

		void SetRenderSize(uint32_t aWidth, uint32_t aHeight);

		Entity CreateEntity(const std::string& tag = "");
		void RemoveEntity(Entity entity);
		void RemoveEntity(Entity entity, float aTimeToDestroy);

		void ParentEntity(Entity parent, Entity child);
		void UnparentEntity(Entity entity);
		
		gem::mat4 GetWorldSpaceTransform(Entity entity, bool accountForActor = false);
		TRS GetWorldSpaceTRS(Entity entity);

		Entity InstantiateSplitMesh(const std::filesystem::path& path);

		gem::vec3 GetWorldForward(Entity entity);
		gem::vec3 GetWorldRight(Entity entity);
		gem::vec3 GetWorldUp(Entity entity);

		template<typename T>
		const std::vector<Wire::EntityId> GetAllEntitiesWith() const;

		static const std::set<AssetHandle> GetDependencyList(const std::filesystem::path& scenePath);
		static bool IsSceneFullyLoaded(const std::filesystem::path& scenePath);
		static void PreloadSceneAssets(const std::filesystem::path& scenePath);

		static AssetType GetStaticType() { return AssetType::Scene; }
		AssetType GetType() override { return GetStaticType(); }

		void CopyTo(Ref<Scene> otherScene);

		Ref<ParticleSystem> myParticleSystem;
	private:
		friend class Entity;
		friend class SceneImporter;

		void SetupComponentCreationFunctions();
		void SetupComponentDeletionFunctions();

		void IsRecursiveChildOf(Entity mainParent, Entity currentEntity, bool& outChild);
		void ConvertToWorldSpace(Entity entity);
		void ConvertToLocalSpace(Entity entity);

		void SortScene();

		SceneEnvironment myEnvironment;
		Statistics myStatistics;

		Ref<PhysicsSystem> myPhysicsSystem;

		bool myIsPlaying = false;
		float myTimeSinceStart = 0.f;

		std::string myName = "New Scene";
		Wire::Registry myRegistry;

		std::map<Wire::EntityId, bool> myEntityTimesToDestroyRemoved;
		std::map<Wire::EntityId, float> myEntityTimesToDestroy;

		uint32_t myWidth = 1;
		uint32_t myHeight = 1;

		uint32_t myLastSortId = 0;
	};

	template<typename T>
	inline const std::vector<Wire::EntityId> Scene::GetAllEntitiesWith() const
	{
		return myRegistry.GetComponentView<T>();
	}
}