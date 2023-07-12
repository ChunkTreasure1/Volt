#include "vtpch.h"
#include "Scene.h"

#include <Volt/Core/Application.h>

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Video/Video.h"

#include "Volt/Core/Profiling.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Scene/EnttEntity.h"

#include "Volt/Components/Components.h"
#include "Volt/Components/AudioComponents.h"
#include "Volt/Components/LightComponents.h"
#include "Volt/Components/CoreComponents.h"

#include "Volt/Animation/AnimationManager.h"
#include "Volt/Animation/AnimationController.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoGCManager.h"

#include "Volt/Utility/FileSystem.h"

#include "Volt/Math/Math.h"

#include "Volt/Rendering/RendererStructs.h"

#include "Volt/Vision/Vision.h"
#include "Volt/Utility/Random.h"

#include "Volt/Discord/DiscordSDK.h"

#include <Wire/Serialization.h>

#include <GraphKey/TimerManager.h>
#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>

#include <Navigation/Core/NavigationSystem.h>

#include <stack>
#include <ranges>

namespace Volt
{
	Scene::Scene(const std::string& name)
		: myName(name), myAnimationSystem(this)
	{
		myVisionSystem = CreateRef<Vision>(this);

		SetupComponentCreationFunctions();
		SetupComponentDeletionFunctions();

		AddLayer("Main", 0);
	}

	Scene::Scene()
		: myAnimationSystem(this)
	{
		myVisionSystem = CreateRef<Vision>(this);

		SetupComponentCreationFunctions();
		SetupComponentDeletionFunctions();

		AddLayer("Main", 0);
	}

	void Scene::OnEvent(Event& e)
	{
		VT_PROFILE_SCOPE((std::string("Scene::OnEvent: ") + std::string(e.GetName())).c_str());

		if (!myIsPlaying)
		{
			return;
		}

		//myRegistry.ForEach<VisualScriptingComponent>([&](Wire::EntityId id, const VisualScriptingComponent& comp)
		//{
		//	if (comp.graph)
		//	{
		//		comp.graph->OnEvent(e);
		//	}
		//});

		myRegistry.ForEach<AnimationControllerComponent>([&](Wire::EntityId id, const AnimationControllerComponent& controller)
		{
			if (controller.controller)
			{
				controller.controller->GetGraph()->OnEvent(e);
			}
		});

		myAudioSystem.OnEvent(myRegistry, e);
		myVisionSystem->OnEvent(e);
	}

	void Scene::SetRenderSize(uint32_t aWidth, uint32_t aHeight)
	{
		myWidth = aWidth;
		myHeight = aHeight;
	}

	void Scene::SetTimeScale(const float aTimeScale)
	{
		Volt::Application::Get().SetTimeScale(aTimeScale);
	}

	void Scene::InitializeEngineScripts()
	{
		MonoScriptEngine::OnRuntimeStart(this);

		myRegistry.ForEachSafe<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp)
		{
			for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				auto scriptClass = MonoScriptEngine::GetScriptClass(scriptComp.scriptNames[i]);

				if (scriptClass && scriptClass->IsEngineScript())
				{
					MonoScriptEngine::OnAwakeInstance(scriptComp.scriptIds[i], id, scriptComp.scriptNames[i]);
				}
			}
		});

		MonoScriptEngine::DoOnAwakeInstance();

		myRegistry.ForEachSafe<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp)
		{
			for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				auto scriptClass = MonoScriptEngine::GetScriptClass(scriptComp.scriptNames[i]);

				if (scriptClass && scriptClass->IsEngineScript())
				{
					MonoScriptEngine::OnCreateInstance(scriptComp.scriptIds[i], id, scriptComp.scriptNames[i]);
				}
			}
		});
	}

	void Scene::ShutdownEngineScripts()
	{
		myRegistry.ForEachSafe<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp)
		{
			for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				auto scriptClass = MonoScriptEngine::GetScriptClass(scriptComp.scriptNames[i]);

				if (scriptClass && scriptClass->IsEngineScript())
				{
					MonoScriptEngine::OnDestroyInstance(scriptComp.scriptIds[i]);
				}
			}
		});

		MonoScriptEngine::OnRuntimeEnd();
	}

	void Scene::OnRuntimeStart()
	{
		Physics::CreateScene(this);
		Physics::CreateActors(this);
		AnimationManager::Reset();

		myIsPlaying = true;
		myTimeSinceStart = 0.f;

		myAudioSystem.RuntimeStart(myRegistry, this);

		Application::Get().GetNavigationSystem().OnRuntimeStart();

		MonoScriptEngine::OnRuntimeStart(this);

		myRegistry.ForEach<EntityDataComponent>([](Wire::EntityId, EntityDataComponent& dataComp)
		{
			dataComp.timeSinceCreation = 0.f;
			dataComp.randomValue = Random::Float(0.f, 1.f);
		});

		myVisionSystem->Initialize();
		myAnimationSystem.OnRuntimeStart(myRegistry);

		myRegistry.ForEachSafe<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp)
		{
			for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				MonoScriptEngine::OnAwakeInstance(scriptComp.scriptIds[i], id, scriptComp.scriptNames[i]);
			}
		});

		MonoScriptEngine::DoOnAwakeInstance();

		myRegistry.ForEachSafe<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp)
		{
			for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				MonoScriptEngine::OnCreateInstance(scriptComp.scriptIds[i], id, scriptComp.scriptNames[i]);
			}
		});

		myRegistry.ForEachSafe<BehaviorTreeComponent>([&](Wire::EntityId id, BehaviorTreeComponent& comp)
		{
			if (comp.treeHandle == Volt::Asset::Null())
				return;
			auto tree = AssetManager::Get().GetAsset<BehaviorTree::Tree>(comp.treeHandle);
			if (!tree)
				return;
			comp.tree = CreateRef<Volt::BehaviorTree::Tree>(*tree);
			comp.tree->SetEntity(Volt::Entity(id, this));
		});
	}

	void Scene::OnRuntimeEnd()
	{
		myIsPlaying = false;
		GraphKey::TimerManager::Clear();

		myRegistry.ForEachSafe<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp)
		{
			for (const auto& instId : scriptComp.scriptIds)
			{
				MonoScriptEngine::OnDestroyInstance(instId);
			}
		});

		MonoScriptEngine::OnRuntimeEnd();

		myAnimationSystem.OnRuntimeEnd(myRegistry);
		Physics::DestroyScene();
		myAudioSystem.RuntimeStop(myRegistry, this);
	}

	void Scene::OnSimulationStart()
	{
		Physics::CreateScene(this);
		Physics::CreateActors(this);
	}

	void Scene::OnSimulationEnd()
	{
		Physics::DestroyScene();
	}

	void Scene::Update(float aDeltaTime)
	{
		VT_PROFILE_FUNCTION();
		myStatistics.entityCount = (uint32_t)myRegistry.GetAllEntities().size();

		AnimationManager::Update(aDeltaTime);
		Physics::GetScene()->Simulate(aDeltaTime);
		myVisionSystem->Update(aDeltaTime);

		myTimeSinceStart += aDeltaTime;
		myCurrentDeltaTime = aDeltaTime;

		// Update scene data
		{
			//SceneData sceneData;
			//sceneData.deltaTime = aDeltaTime;
			//sceneData.timeSinceStart = myTimeSinceStart;

			//Renderer::SetSceneData(sceneData);
		}

		GraphKey::TimerManager::Update(aDeltaTime);

		{
			VT_PROFILE_SCOPE("Update entity time");

			const auto& dataCompView = myRegistry.GetSingleComponentView<EntityDataComponent>();
			for (const auto& entId : dataCompView)
			{
				auto& dataComp = myRegistry.GetComponent<EntityDataComponent>(entId);
				dataComp.timeSinceCreation += aDeltaTime;
			}
		}

		MonoScriptEngine::DoDestroyQueue();
		MonoScriptEngine::OnUpdate(aDeltaTime);
		myRegistry.ForEachSafe<MonoScriptComponent, TransformComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp, const TransformComponent& transformComponent)
		{
			if (!transformComponent.visible)
			{
				return;
			}

			for (size_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				VT_PROFILE_SCOPE(std::string("Updating script" + scriptComp.scriptNames.at(i)).c_str());
				MonoScriptEngine::OnUpdateInstance(scriptComp.scriptIds.at(i), aDeltaTime);
			}
		});
		MonoScriptEngine::OnUpdateEntity(aDeltaTime);

		myRegistry.ForEach<CameraComponent, TransformComponent>([this](Wire::EntityId id, CameraComponent& cameraComp, const TransformComponent& transComp)
		{
			if (transComp.visible)
			{
				Entity entity = { id, this };
				 
				cameraComp.camera->SetPerspectiveProjection(cameraComp.fieldOfView, (float)myWidth / (float)myHeight, cameraComp.nearPlane, cameraComp.farPlane);
				cameraComp.camera->SetPosition(entity.GetPosition());
				cameraComp.camera->SetRotation(glm::eulerAngles(entity.GetRotation()));

				cameraComp.camera->SetAperture(cameraComp.aperture);
				cameraComp.camera->SetShutterSpeed(cameraComp.shutterSpeed);
				cameraComp.camera->SetISO(cameraComp.iso);
			}
		});

		//myRegistry.ForEach<BehaviorTreeComponent>([&](Wire::EntityId, BehaviorTreeComponent& behaviorTree)
		//{
		//	if (behaviorTree.tree)
		//	{
		//		behaviorTree.tree->Run();
		//	}
		//});

		//myRegistry.ForEach<VideoPlayerComponent>([&](Wire::EntityId, VideoPlayerComponent& videoPlayer)
		//{
		//	if (videoPlayer.lastVideoHandle != videoPlayer.videoHandle)
		//	{
		//		Ref<Video> video = AssetManager::GetAsset<Video>(videoPlayer.videoHandle);
		//		if (video && video->IsValid())
		//		{
		//			videoPlayer.videoHandle = video->handle;
		//			videoPlayer.lastVideoHandle = videoPlayer.videoHandle;

		//			video->Play(true);
		//		}
		//	}

		//	if (videoPlayer.videoHandle != Asset::Null())
		//	{
		//		Ref<Video> videoAsset = AssetManager::GetAsset<Video>(videoPlayer.videoHandle);
		//		if (videoAsset && videoAsset->IsValid())
		//		{
		//			videoAsset->Update(aDeltaTime);
		//		}
		//	}
		//});

		myParticleSystem.Update(myRegistry, this, aDeltaTime);
		myAudioSystem.Update(myRegistry, this, aDeltaTime);
		myAnimationSystem.Update(myRegistry, aDeltaTime);

		for (auto& [ent, time] : myEntityTimesToDestroy)
		{
			time -= aDeltaTime;
			if (time <= 0.f && !myEntityTimesToDestroyRemoved.at(ent))
			{
				RemoveEntity(Entity{ ent, this });
			}
		}

		for (auto it = myEntityTimesToDestroyRemoved.begin(); it != myEntityTimesToDestroyRemoved.end();)
		{
			if (it->second == true)
			{
				myEntityTimesToDestroy.erase(it->first);
				it = myEntityTimesToDestroyRemoved.erase(it);
			}
			else
			{
				++it;
			}
		}

		for (auto it = myEntityTimesToDestroy.begin(); it != myEntityTimesToDestroy.end();)
		{
			if (it->second <= 0.f)
			{
				it = myEntityTimesToDestroy.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void Scene::FixedUpdate(float aDeltaTime)
	{
	}

	void Scene::UpdateEditor(float aDeltaTime)
	{
		myStatistics.entityCount = (uint32_t)myRegistry.GetAllEntities().size();
		myParticleSystem.Update(myRegistry, this, aDeltaTime);

		// Update scene data
		{
			SceneData sceneData;
			sceneData.deltaTime = aDeltaTime;
			sceneData.timeSinceStart = myTimeSinceStart;

			//Renderer::SetSceneData(sceneData);
		}

		myRegistry.ForEach<CameraComponent, TransformComponent>([this](Wire::EntityId, CameraComponent& cameraComp, const TransformComponent& transComp)
		{
			if (transComp.visible)
			{
				cameraComp.camera->SetPerspectiveProjection(cameraComp.fieldOfView, (float)myWidth / (float)myHeight, cameraComp.nearPlane, cameraComp.farPlane);
				cameraComp.camera->SetPosition(transComp.position);
				cameraComp.camera->SetRotation(glm::eulerAngles(transComp.rotation));
			}
		});
	}

	void Scene::UpdateSimulation(float aDeltaTime)
	{
		Physics::GetScene()->Simulate(aDeltaTime);

		myStatistics.entityCount = (uint32_t)myRegistry.GetAllEntities().size();

		// Update scene data
		{
			// #TODO_Ivar: Reimplement

			SceneData sceneData;
			sceneData.deltaTime = aDeltaTime;
			sceneData.timeSinceStart = myTimeSinceStart;
		}
	}

	Entity Scene::CreateEntity(const std::string& tag)
	{
		Wire::EntityId id = myRegistry.CreateEntity();

		Entity newEntity = Entity(id, this);
		auto& transform = newEntity.AddComponent<TransformComponent>();
		transform.position = { 0.f, 0.f, 0.f };
		transform.rotation = { 1.f, 0.f, 0.f, 0.f };
		transform.scale = { 1.f, 1.f, 1.f };

		newEntity.AddComponent<TagComponent>(tag.empty() ? "New Entity" : tag);
		newEntity.AddComponent<EntityDataComponent>();
		newEntity.AddComponent<VisualScriptingComponent>();
		newEntity.AddComponent<RelationshipComponent>();

		newEntity.GetComponent<EntityDataComponent>().layerId = mySceneLayers.at(myActiveLayerIndex).id;
		newEntity.GetComponent<EntityDataComponent>().randomValue = Random::Float(0.f, 1.f);
		newEntity.GetComponent<EntityDataComponent>().timeSinceCreation = 0.f;

		InvalidateEntityTransform(id);

		SortScene();
		return newEntity;
	}

	void Scene::RemoveEntity(Entity entity)
	{
		if (!myRegistry.Exists(entity.GetId()))
		{
			return;
		}

		if (myEntityTimesToDestroyRemoved.contains(entity.GetId()))
		{
			myEntityTimesToDestroyRemoved.at(entity.GetId()) = true;
		}

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& relComp = entity.GetComponent<RelationshipComponent>();
			if (relComp.Parent)
			{
				Entity parentEnt{ relComp.Parent, this };

				if (parentEnt.HasComponent<RelationshipComponent>())
				{
					auto& parentRelComp = parentEnt.GetComponent<RelationshipComponent>();
					auto it = std::find(parentRelComp.Children.begin(), parentRelComp.Children.end(), entity.GetId());
					if (it != parentRelComp.Children.end())
					{
						parentRelComp.Children.erase(it);
					}
				}
			}

			for (int32_t i = (int32_t)relComp.Children.size() - 1; i >= 0; --i)
			{
				Entity childEnt{ relComp.Children.at(i), this };
				RemoveEntity(childEnt);
			}
		}

		if (entity.HasComponent<MonoScriptComponent>())
		{
			for (const auto& scriptId : entity.GetComponent<MonoScriptComponent>().scriptIds)
			{
				MonoScriptEngine::OnDestroyInstance(scriptId);
			}
		}

		myRegistry.RemoveEntity(entity.GetId());
		SortScene();
	}

	void Scene::RemoveEntity(Entity entity, float aTimeToDestroy)
	{
		if (myEntityTimesToDestroy.find(entity.GetId()) != myEntityTimesToDestroy.end())
		{
			return;
		}

		myEntityTimesToDestroy.emplace(entity.GetId(), aTimeToDestroy);
		myEntityTimesToDestroyRemoved.emplace(entity.GetId(), false);
	}

	void Scene::ParentEntity(Entity parent, Entity child)
	{
		if (parent.IsNull() || child.IsNull() || parent == child)
		{
			return;
		}

		// Check that it's not in the child chain
		bool isChild = false;
		IsRecursiveChildOf(parent, child, isChild);
		if (isChild)
		{
			return;
		}

		if (child.GetParent())
		{
			UnparentEntity(child);
		}

		auto& childChildren = child.GetComponent<RelationshipComponent>().Children;

		if (auto it = std::find(childChildren.begin(), childChildren.end(), parent.GetId()) != childChildren.end())
		{
			return;
		}

		child.GetComponent<RelationshipComponent>().Parent = parent.GetId();
		parent.GetComponent<RelationshipComponent>().Children.emplace_back(child.GetId());

		if (child.GetLayerId() != parent.GetLayerId())
		{
			MoveToLayer(child, parent.GetLayerId());
		}

		ConvertToLocalSpace(child);
	}

	void Scene::UnparentEntity(Entity entity)
	{
		if (entity.IsNull()) { return; }

		auto parent = entity.GetParent();
		if (parent.IsNull())
		{
			return;
		}

		auto& children = parent.GetComponent<RelationshipComponent>().Children;

		auto it = std::find(children.begin(), children.end(), entity.GetId());
		if (it != children.end())
		{
			children.erase(it);
		}

		ConvertToWorldSpace(entity);
		entity.GetComponent<RelationshipComponent>().Parent = Wire::NullID;
	}

	glm::mat4 Scene::GetWorldSpaceTransform(Entity entity)
	{
		{
			std::shared_lock lock(myCachedEntityTransformMutex);
			if (myCachedEntityTransforms.contains(entity.GetId()))
			{
				return myCachedEntityTransforms.at(entity.GetId());
			}
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return { 1.f };
		}

		const auto tqs = GetWorldSpaceTRS(entity);
		const glm::mat4 transform = glm::translate(glm::mat4{1.f}, tqs.position)* glm::mat4_cast(tqs.rotation)* glm::scale(glm::mat4{ 1.f }, tqs.scale);

		{
			std::unique_lock lock{ myCachedEntityTransformMutex };
			myCachedEntityTransforms[entity.GetId()] = transform;
		}

		return transform;
	}

	Scene::TQS Scene::GetWorldSpaceTRS(Entity entity)
	{
		if (!entity.HasComponent<TransformComponent>())
		{
			return {};
		}

		TQS transform;

		Entity parent = entity.GetParent();
		if (parent)
		{
			transform = GetWorldSpaceTRS(parent);
		}

		const auto& transComp = entity.GetComponent<TransformComponent>();

		transform.position = transform.position + transform.rotation * transComp.position;
		transform.rotation = transform.rotation * transComp.rotation;
		transform.scale = transform.scale * transComp.scale;

		return transform;
	}

	void Scene::InvalidateEntityTransform(Wire::EntityId entity)
	{
		std::unique_lock lock{ myCachedEntityTransformMutex };

		std::vector<Wire::EntityId> entityStack;
		entityStack.reserve(10);
		entityStack.push_back(entity);

		while (!entityStack.empty())
		{
			Wire::EntityId currentEntity = entityStack.back();
			entityStack.pop_back();

			Volt::Entity ent{ currentEntity, this };

			if (!ent.HasComponent<RelationshipComponent>())
			{
				continue;
			}

			auto& relComp = ent.GetComponent<RelationshipComponent>();

			for (const auto& child : relComp.Children)
			{
				entityStack.push_back(child);
			}

			if (myCachedEntityTransforms.contains(currentEntity))
			{
				myCachedEntityTransforms.erase(currentEntity);
			}
		}
	}

	Entity Scene::InstantiateSplitMesh(AssetHandle meshHandle)
	{
		auto mesh = AssetManager::GetAsset<Mesh>(meshHandle);
		if (!mesh || !mesh->IsValid())
		{
			VT_CORE_WARN("Trying to instantiate invalid mesh {0}!", meshHandle);
			return Entity{};
		}

		Entity parentEntity = CreateEntity();

		for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
		{
			subMesh;
			Entity childEntity = CreateEntity();
			ParentEntity(parentEntity, childEntity);

			auto& meshComponent = childEntity.AddComponent<MeshComponent>();
			meshComponent.handle = mesh->handle;
			//meshComponent.subMeshIndex = i;
			i++;
		}

		return parentEntity;
	}

	glm::vec3 Scene::GetWorldForward(Entity entity)
	{
		return glm::rotate(entity.GetRotation(), glm::vec3{ 0.f, 0.f, 1.f });
	}

	glm::vec3 Scene::GetWorldRight(Entity entity)
	{
		return glm::rotate(entity.GetRotation(), glm::vec3{ 1.f, 0.f, 0.f });
	}

	glm::vec3 Scene::GetWorldUp(Entity entity)
	{
		return glm::rotate(entity.GetRotation(), glm::vec3{ 0.f, 1.f, 0.f });
	}

	const Entity Scene::GetEntityWithName(std::string name)
	{
		Entity entity;

		myRegistry.ForEach<TagComponent>([&](Wire::EntityId id, TagComponent& comp)
		{
			if (comp.tag == name)
			{
				entity = { id, this };
			}
		});

		return entity;
	}

	const std::set<AssetHandle> Scene::GetDependencyList(const std::filesystem::path& scenePath)
	{
		const auto folderPath = scenePath.parent_path();
		const auto depPath = ProjectManager::GetProjectDirectory() / folderPath / "Dependencies.vtdep";

		if (!FileSystem::Exists(depPath))
		{
			return {}; // Assume scene is loaded if there is no dependency list
		}

		std::set<AssetHandle> dependencies;

		// Load dependencies
		{
			std::ifstream file(depPath);
			if (!file.is_open())
			{
				VT_CORE_ERROR("[AssetManager] Unable to read dependency file!");
				return {};
			}

			std::stringstream strStream;
			strStream << file.rdbuf();
			file.close();

			YAML::Node root;
			try
			{
				root = YAML::Load(strStream.str());
			}
			catch (std::exception& e)
			{
				VT_CORE_ERROR("[AssetManager] Dependency list contains invalid YAML! Please correct it! Error: {0}", e.what());
				return {};
			}

			YAML::Node depsNode = root["Dependencies"];
			for (const auto entry : depsNode)
			{
				dependencies.emplace(entry["Handle"].as<uint64_t>());
			}
		}

		return dependencies;
	}

	bool Scene::IsSceneFullyLoaded(const std::filesystem::path& scenePath)
	{
		if (!FileSystem::Exists(ProjectManager::GetProjectDirectory() / scenePath))
		{
			return false;
		}

		const std::set<AssetHandle> dependencies = GetDependencyList(scenePath);

		bool result = true;
		for (const auto& dep : dependencies)
		{
			if (AssetManager::Get().IsLoaded(dep))
			{
				Ref<Asset> rawAsset = AssetManager::Get().GetAssetRaw(dep);
				result &= !rawAsset->IsFlagSet(AssetFlag::Queued);
			}
		}

		return result;
	}

	void Scene::PreloadSceneAssets(const std::filesystem::path& scenePath)
	{
		if (!FileSystem::Exists(ProjectManager::GetProjectDirectory() / scenePath))
		{
			return;
		}

		const std::set<AssetHandle> dependencies = GetDependencyList(scenePath);
		for (const auto& dep : dependencies)
		{
			AssetManager::Get().QueueAssetRaw(dep);
		}

		VT_CORE_INFO("[Scene] {0} assets has been queued for scene {1}!", dependencies.size(), scenePath.string());
	}

	Ref<Scene> Scene::CreateDefaultScene(const std::string& name, bool createDefaultMesh)
	{
		Ref<Scene> newScene = CreateRef<Scene>(name);

		// Setup
		{
			// Cube
			if (createDefaultMesh)
			{
				auto ent = newScene->CreateEntity("Cube");
				auto& meshComp = ent.AddComponent<MeshComponent>();
				meshComp.handle = AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cube.vtmesh");
			}

			// Light
			{
				auto ent = newScene->CreateEntity();
				ent.SetTag("Directional Light");
				ent.AddComponent<DirectionalLightComponent>();

				auto& trans = ent.GetComponent<TransformComponent>();
				trans.rotation = glm::quat{ glm::vec3{ glm::radians(120.f), 0.f, 0.f } };
			}

			// Skylight
			{
				auto ent = newScene->CreateEntity("Skylight");
				ent.AddComponent<SkylightComponent>();
			}

			// Camera
			{
				auto ent = newScene->CreateEntity("Camera");
				ent.AddComponent<CameraComponent>();

				ent.SetPosition({ 0.f, 0.f, -500.f });
			}
		}

		return newScene;
	}

	void Scene::CopyTo(Ref<Scene> otherScene)
	{
		VT_PROFILE_FUNCTION();

		otherScene->myName = myName;
		otherScene->myEnvironment = myEnvironment;
		otherScene->handle = handle;
		otherScene->mySceneLayers = mySceneLayers;
		otherScene->myActiveLayerIndex = myActiveLayerIndex;
		otherScene->myLastLayerId = myLastLayerId;

		auto& otherRegistry = otherScene->GetRegistry();

		// Copy registry
		for (const auto& ent : myRegistry.GetAllEntities())
		{
			otherRegistry.AddEntity(ent);
			Entity::Copy(myRegistry, otherRegistry, myMonoFieldCache, otherScene->GetScriptFieldCache(), ent, ent);
		}

		otherRegistry.ForEach<VisualScriptingComponent>([&](Wire::EntityId id, VisualScriptingComponent& comp)
		{
			if (!comp.graph)
			{
				return;
			}

			// Set entities to correct scene
			{
				for (const auto& n : comp.graph->GetNodes())
				{
					for (auto& a : n->inputs)
					{
						if (a.data.has_value() && a.data.type() == typeid(Volt::Entity))
						{
							Volt::Entity ent = std::any_cast<Volt::Entity>(a.data);
							a.data = Volt::Entity{ ent.GetId(), otherScene.get() };
						}
					}

					for (auto& a : n->outputs)
					{
						if (a.data.has_value() && a.data.type() == typeid(Volt::Entity))
						{
							Volt::Entity ent = std::any_cast<Volt::Entity>(a.data);
							a.data = Volt::Entity{ ent.GetId(), otherScene.get() };
						}
					}
				}
			}
		});
	}

	void Scene::Clear()
	{
		myRegistry.Clear();
	}

	const glm::vec3 Scene::GetWorldPosition(EnttEntity entity) const
	{
		std::vector<EnttEntity> hierarchy{};
		hierarchy.emplace_back(entity);

		EnttEntity currentEntity = entity;
		while (currentEntity.HasParent())
		{
			auto parent = currentEntity.GetParent();
			hierarchy.emplace_back(parent);
			currentEntity = parent;
		}

		glm::vec3 resultPosition = 0.f;
		for (const auto& parent : std::ranges::reverse_view(hierarchy))
		{
			const auto& transComp = m_enttRegistry.get<EnTTTransformComponent>(parent.GetId());
			resultPosition = resultPosition + transComp.position * transComp.rotation;
		}

		return resultPosition;
	}

	const glm::quat Scene::GetWorldRotation(EnttEntity entity) const
	{
		std::vector<EnttEntity> hierarchy{};
		hierarchy.emplace_back(entity);

		EnttEntity currentEntity = entity;
		while (currentEntity.HasParent())
		{
			auto parent = currentEntity.GetParent();
			hierarchy.emplace_back(parent);
			currentEntity = parent;
		}

		glm::quat resultRotation = glm::identity<glm::quat>();
		for (const auto& parent : std::ranges::reverse_view(hierarchy))
		{
			const auto& transComp = m_enttRegistry.get<EnTTTransformComponent>(parent.GetId());
			resultRotation = resultRotation * transComp.rotation;
		}

		return resultRotation;
	}

	const glm::vec3 Scene::GetWorldScale(EnttEntity entity) const
	{
		std::vector<EnttEntity> hierarchy{};
		hierarchy.emplace_back(entity);

		EnttEntity currentEntity = entity;
		while (currentEntity.HasParent())
		{
			auto parent = currentEntity.GetParent();
			hierarchy.emplace_back(parent);
			currentEntity = parent;
		}

		glm::vec3 resultScale = 0.f;
		for (const auto& parent : std::ranges::reverse_view(hierarchy))
		{
			const auto& transComp = m_enttRegistry.get<EnTTTransformComponent>(parent.GetId());
			resultScale = resultScale + transComp.scale;
		}

		return resultScale;
	}

	const glm::mat4 Scene::GetWorldTransform(EnttEntity entity) const
	{
		std::vector<EnttEntity> hierarchy{};
		hierarchy.emplace_back(entity);

		EnttEntity currentEntity = entity;
		while (currentEntity.HasParent())
		{
			auto parent = currentEntity.GetParent();
			hierarchy.emplace_back(parent);
			currentEntity = parent;
		}

		glm::mat4 resultTransform = glm::identity<glm::mat4>();
		for (const auto& parent : std::ranges::reverse_view(hierarchy))
		{
			const auto& transComp = m_enttRegistry.get<EnTTTransformComponent>(parent.GetId());
			resultTransform = resultTransform * transComp.GetTransform();
		}

		return resultTransform;
	}

	const Scene::TQS Scene::GetWorldTQS(EnttEntity entity) const
	{
		std::vector<EnttEntity> hierarchy{};
		hierarchy.emplace_back(entity);

		EnttEntity currentEntity = entity;
		while (currentEntity.HasParent())
		{
			auto parent = currentEntity.GetParent();
			hierarchy.emplace_back(parent);
			currentEntity = parent;
		}

		TQS resultTransform{};
		for (const auto& parent : std::ranges::reverse_view(hierarchy))
		{
			const auto& transComp = m_enttRegistry.get<EnTTTransformComponent>(parent.GetId());
		
			resultTransform.position = resultTransform.position + transComp.rotation * transComp.position;
			resultTransform.rotation = resultTransform.rotation * transComp.rotation;
			resultTransform.scale = resultTransform.scale * transComp.scale;
		}

		return resultTransform;
	}

	void Scene::MoveToLayerRecursive(Entity entity, uint32_t targetLayer)
	{
		if (!entity.HasComponent<EntityDataComponent>())
		{
			return;
		}

		entity.GetComponent<EntityDataComponent>().layerId = targetLayer;

		for (const auto& child : entity.GetChilden())
		{
			MoveToLayerRecursive(child, targetLayer);
		}
	}

	void Scene::SetupComponentCreationFunctions()
	{
		myRegistry.SetOnCreateFunction<RigidbodyComponent>([&](Wire::EntityId id, void*)
		{
			if (!myIsPlaying)
			{
				return;
			}

			Physics::CreateActor(Entity{ id, this });
		});

		myRegistry.SetOnCreateFunction<CharacterControllerComponent>([&](Wire::EntityId id, void*)
		{
			if (!myIsPlaying)
			{
				return;
			}

			Physics::CreateControllerActor(Entity{ id, this });
		});

		myRegistry.SetOnCreateFunction<BoxColliderComponent>([&](Wire::EntityId id, void* compPtr)
		{
			auto entity = Entity{ id, this };

			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(entity);
			BoxColliderComponent& comp = *static_cast<BoxColliderComponent*>(compPtr);

			if (actor && !comp.added)
			{
				actor->AddCollider(comp, entity);
			}
		});

		myRegistry.SetOnCreateFunction<SphereColliderComponent>([&](Wire::EntityId id, void* compPtr)
		{
			auto entity = Entity{ id, this };

			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(entity);
			SphereColliderComponent& comp = *static_cast<SphereColliderComponent*>(compPtr);

			if (actor && !comp.added)
			{
				actor->AddCollider(comp, entity);
			}
		});

		myRegistry.SetOnCreateFunction<CapsuleColliderComponent>([&](Wire::EntityId id, void* compPtr)
		{
			auto entity = Entity{ id, this };

			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(entity);
			CapsuleColliderComponent& comp = *static_cast<CapsuleColliderComponent*>(compPtr);

			if (actor && !comp.added)
			{
				actor->AddCollider(comp, entity);
			}
		});

		myRegistry.SetOnCreateFunction<MeshColliderComponent>([&](Wire::EntityId id, void* compPtr)
		{
			auto entity = Entity{ id, this };

			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(entity);
			MeshColliderComponent& comp = *static_cast<MeshColliderComponent*>(compPtr);

			if (actor && !comp.added)
			{
				actor->AddCollider(comp, entity);
			}
		});

		myRegistry.SetOnCreateFunction<AudioSourceComponent>([&](Wire::EntityId id, void* compPtr)
		{
			if (!myIsPlaying)
			{
				return;
			}

			AudioSourceComponent& comp = *static_cast<AudioSourceComponent*>(compPtr);
			comp.OnCreate(id);
		});

		myRegistry.SetOnCreateFunction<AudioListenerComponent>([&](Wire::EntityId id, void* compPtr)
		{
			if (!myIsPlaying)
			{
				return;
			}

			AudioListenerComponent& comp = *static_cast<AudioListenerComponent*>(compPtr);
			comp.OnCreate(id);
		});


	}
	void Scene::SetupComponentDeletionFunctions()
	{
		myRegistry.SetOnRemoveFunction<RigidbodyComponent>([&](Wire::EntityId id, void*)
		{
			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
			if (actor)
			{
				Physics::GetScene()->RemoveActor(actor);
			}
		});

		myRegistry.SetOnRemoveFunction<CharacterControllerComponent>([&](Wire::EntityId id, void*)
		{
			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetControllerActor(Entity{ id, this });
			if (actor)
			{
				Physics::GetScene()->RemoveControllerActor(actor);
			}
		});

		myRegistry.SetOnRemoveFunction<BoxColliderComponent>([&](Wire::EntityId id, void*)
		{
			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
			if (actor)
			{
				actor->RemoveCollider(ColliderType::Box);
			}
		});

		myRegistry.SetOnRemoveFunction<SphereColliderComponent>([&](Wire::EntityId id, void*)
		{
			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
			if (actor)
			{
				actor->RemoveCollider(ColliderType::Sphere);
			}
		});

		myRegistry.SetOnRemoveFunction<CapsuleColliderComponent>([&](Wire::EntityId id, void*)
		{
			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
			if (actor)
			{
				actor->RemoveCollider(ColliderType::Capsule);
			}
		});

		myRegistry.SetOnRemoveFunction<MeshColliderComponent>([&](Wire::EntityId id, void* compPtr)
		{
			if (!myIsPlaying)
			{
				return;
			}

			auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
			if (actor)
			{
				auto& comp = *static_cast<MeshColliderComponent*>(compPtr);
				if (comp.isConvex)
				{
					actor->RemoveCollider(ColliderType::ConvexMesh);
				}
				else
				{
					actor->RemoveCollider(ColliderType::TriangleMesh);
				}
			}
		});
	}

	void Scene::IsRecursiveChildOf(Entity parent, Entity currentEntity, bool& outChild)
	{
		if (currentEntity.HasComponent<RelationshipComponent>())
		{
			auto& relComp = currentEntity.GetComponent<RelationshipComponent>();
			for (const auto& child : relComp.Children)
			{
				outChild |= parent.GetId() == child;
				IsRecursiveChildOf(parent, Entity{ child, this }, outChild);
			}
		}
	}

	void Scene::ConvertToWorldSpace(Entity entity)
	{
		Entity parent = entity.GetParent();

		if (!parent)
		{
			return;
		}

		auto& transform = entity.GetComponent<TransformComponent>();

		const glm::mat4 transformMatrix = GetWorldSpaceTransform(entity);

		glm::vec3 r;
		Math::Decompose(transformMatrix, transform.position, r, transform.scale);

		transform.rotation = glm::quat{ r };

		InvalidateEntityTransform(entity.GetId());
	}

	void Scene::ConvertToLocalSpace(Entity entity)
	{
		Entity parent = entity.GetParent();

		if (!parent)
		{
			return;
		}

		auto& transform = entity.GetComponent<TransformComponent>();
		const glm::mat4 parentTransform = GetWorldSpaceTransform(parent);
		const glm::mat4 localTransform = glm::inverse(parentTransform) * transform.GetTransform();

		glm::vec3 r;
		Math::Decompose(localTransform, transform.position, r, transform.scale);
		transform.rotation = glm::quat{ r };

		InvalidateEntityTransform(entity.GetId());
	}

	void Scene::AddLayer(const std::string& layerName, uint32_t layerId)
	{
		mySceneLayers.emplace_back(layerId, layerName);
	}

	void Scene::SortScene()
	{
		myRegistry.Sort([](const auto& lhs, const auto& rhs)
		{
			return lhs < rhs;
		});
	}

	void Scene::AddLayer(const std::string& layerName)
	{
		mySceneLayers.emplace_back(myLastLayerId++, layerName);
	}

	void Scene::RemoveLayer(const std::string& layerName)
	{
		mySceneLayers.erase(std::remove_if(mySceneLayers.begin(), mySceneLayers.end(), [&](const SceneLayer& lhs)
		{
			return lhs.name == layerName;
		}), mySceneLayers.end());
	}

	void Scene::RemoveLayer(uint32_t layerId)
	{
		mySceneLayers.erase(std::remove_if(mySceneLayers.begin(), mySceneLayers.end(), [&](const SceneLayer& lhs)
		{
			return lhs.id == layerId;
		}), mySceneLayers.end());
	}

	void Scene::MoveToLayer(Entity entity, uint32_t targetLayer)
	{
		if (entity.GetParent())
		{
			if (entity.GetParent().GetLayerId() != targetLayer)
			{
				entity.ResetParent();
			}
		}

		MoveToLayerRecursive(entity, targetLayer);
	}

	void Scene::SetActiveLayer(uint32_t layerId)
	{
		auto it = std::find_if(mySceneLayers.begin(), mySceneLayers.end(), [&](const auto& lhs) { return lhs.id == layerId; });

		if (it != mySceneLayers.end())
		{
			myActiveLayerIndex = (uint32_t)std::distance(mySceneLayers.begin(), it);
		}

		auto& act = Volt::DiscordSDK::GetRichPresence();

		act.GetParty().GetSize().SetCurrentSize(GetActiveLayer() + 1);
		act.GetParty().GetSize().SetMaxSize(static_cast<int32_t>(GetLayers().size()));

		Volt::DiscordSDK::UpdateRichPresence();
	}

	bool Scene::LayerExists(uint32_t layerId)
	{
		return std::find_if(mySceneLayers.begin(), mySceneLayers.end(), [layerId](const auto& lhs) { return lhs.id == layerId; }) != mySceneLayers.end();
	}
}
