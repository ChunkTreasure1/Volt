#include "vtpch.h"
#include "Scene.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Video/Video.h"
#include "Volt/Animation/AnimationTreeController.h"

#include "Volt/core/Profiling.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Components/Components.h"
#include "Volt/Animation/AnimationManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Physics/PhysicsSystem.h"
#include "Volt/Particles/ParticleSystem.h"

#include "Volt/Scripting/ScriptEngine.h"
#include "Volt/Scripting/Script.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include "Volt/Math/MatrixUtilities.h"
#include "Volt/Utility/FileSystem.h"

#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/Renderer.h"

#include <Wire/Serialization.h>

#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>

namespace Volt
{
	Scene::Scene(const std::string& name)
		: myName(name)
	{
		myParticleSystem = CreateRef<ParticleSystem>(this);

		SetupComponentCreationFunctions();
		SetupComponentDeletionFunctions();
	}

	Scene::Scene()
	{
		myParticleSystem = CreateRef<ParticleSystem>(this);

		SetupComponentCreationFunctions();
		SetupComponentDeletionFunctions();
	}

	void Scene::OnEvent(Event& e)
	{
		if (!myIsPlaying)
		{
			return;
		}

		myRegistry.ForEach<ScriptComponent>([&](Wire::EntityId id, const ScriptComponent& scriptComp)
			{
				for (const auto& scriptId : scriptComp.scripts)
				{
					Ref<Script> scriptInstance = ScriptEngine::GetScript(id, scriptId);

					if (scriptInstance)
					{
						scriptInstance->OnEvent(e);
					}
				}
			});

		myRegistry.ForEach<VisualScriptingComponent>([&](Wire::EntityId id, const VisualScriptingComponent& comp)
			{
				if (comp.graph)
				{
					comp.graph->OnEvent(e);
				}
			});
	}

	void Scene::SetRenderSize(uint32_t aWidth, uint32_t aHeight)
	{
		myWidth = aWidth;
		myHeight = aHeight;
	}

	void Scene::OnRuntimeStart()
	{
		Physics::CreateScene(this);
		Physics::CreateActors(this);
		AnimationManager::Reset();

		myPhysicsSystem = CreateRef<PhysicsSystem>(this);

		myIsPlaying = true;
		myTimeSinceStart = 0.f;

#ifdef VT_ENABLE_MONO	
		MonoScriptEngine::OnRuntimeStart(this);
#endif

		myRegistry.ForEach<EntityDataComponent>([](Wire::EntityId, EntityDataComponent& dataComp)
			{
				dataComp.timeSinceCreation = 0.f;
			});

		myRegistry.ForEach<ScriptComponent>([this](Wire::EntityId id, const ScriptComponent& scriptComp)
			{
				for (const auto& scriptId : scriptComp.scripts)
				{
					Ref<Script> scriptInstance = ScriptRegistry::Create(ScriptRegistry::GetNameFromGUID(scriptId), Entity{ id, this });
					if (!scriptInstance)
					{
						VT_CORE_WARN("Unable to create script with name {0} on entity {1}!", ScriptRegistry::GetNameFromGUID(scriptId), id);
					}
					ScriptEngine::RegisterToEntity(scriptInstance, id);

					if (scriptInstance)
					{
						scriptInstance->OnAwake();
					}
					else
					{
						VT_CORE_WARN("Unable to create script with name {0} on entity {1}!", ScriptRegistry::GetNameFromGUID(scriptId), id);
					}

				}
			});

		myRegistry.ForEach<AnimatedCharacterComponent>([](Wire::EntityId, AnimatedCharacterComponent& animCharComp)
			{
				animCharComp.currentStartTime = AnimationManager::globalClock;

				auto character = AssetManager::GetAsset<AnimatedCharacter>(animCharComp.animatedCharacter);
				if (character && character->IsValid())
				{
					animCharComp.characterStateMachine = CreateRef<AnimationStateMachine>(character);
				}
			});

		myRegistry.ForEach<ScriptComponent>([](Wire::EntityId id, const ScriptComponent& scriptComp)
			{
				for (const auto& scriptId : scriptComp.scripts)
				{
					auto script = ScriptEngine::GetScript(id, scriptId);

					if (script)
					{
						script->OnStart();
					}
					else
					{
						VT_CORE_WARN("Unable to get script with name {0}!", ScriptRegistry::GetNameFromGUID(scriptId));
					}
				}
			});

		myRegistry.ForEach<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp)
			{
				MonoScriptEngine::OnCreateEntityInstance(id, scriptComp.script);
			});

		myRegistry.ForEach<AnimationControllerComponent>([](Wire::EntityId, AnimationControllerComponent& animaitonTree)
			{
				//Create Tree
				animaitonTree.AnimTreeControl = CreateRef<AnimationTreeController>(animaitonTree.animationTreeFile);
			});
	}

	void Scene::OnRuntimeEnd()
	{
		myIsPlaying = false;

		myRegistry.ForEach<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent&)
			{
				MonoScriptEngine::OnDestroyEntityInstance(id);
			});

#ifdef VT_ENABLE_MONO
		MonoScriptEngine::OnRuntimeEnd();
#endif

		myRegistry.ForEach<ScriptComponent>([](Wire::EntityId id, const ScriptComponent& scriptComp)
			{
				for (const auto& scriptId : scriptComp.scripts)
				{
					auto script = ScriptEngine::GetScript(id, scriptId);
					script->OnDetach();
					script->OnStop();
				}
			});

		ScriptEngine::Clear();

		myPhysicsSystem = nullptr;
		Physics::DestroyScene();
	}

	void Scene::OnSimulationStart()
	{
		Physics::CreateScene(this);
		Physics::CreateActors(this);

		myPhysicsSystem = CreateRef<PhysicsSystem>(this);
	}

	void Scene::OnSimulationEnd()
	{
		myPhysicsSystem = nullptr;
		Physics::DestroyScene();
	}

	void Scene::Update(float aDeltaTime)
	{
		VT_PROFILE_FUNCTION();
		myStatistics.entityCount = (uint32_t)myRegistry.GetAllEntities().size();

		AnimationManager::Update(aDeltaTime);
		Physics::GetScene()->Simulate(aDeltaTime);
		myPhysicsSystem->Update(aDeltaTime);

		myTimeSinceStart += aDeltaTime;

		// Update scene data
		{
			SceneData sceneData;
			sceneData.deltaTime = aDeltaTime;
			sceneData.timeSinceStart = myTimeSinceStart;

			Renderer::SetSceneData(sceneData);
		}

		myRegistry.ForEach<EntityDataComponent>([aDeltaTime](Wire::EntityId, EntityDataComponent& dataComp)
			{
				dataComp.timeSinceCreation += aDeltaTime;
			});

		myRegistry.ForEach<ScriptComponent>([&](Wire::EntityId id, const ScriptComponent& scriptComp)
			{
				for (const auto& scriptId : scriptComp.scripts)
				{
					auto script = ScriptEngine::GetScript(id, scriptId);

					if (script)
					{
						script->OnUpdate(aDeltaTime);
					}
					else
					{
						Ref<Script> scriptInstance = ScriptRegistry::Create(ScriptRegistry::GetNameFromGUID(scriptId), Entity{ id, this });
						if (!scriptInstance)
						{
							VT_CORE_WARN("Unable to create script with name {0} on entity {1}!", ScriptRegistry::GetNameFromGUID(scriptId), id);
						}
						ScriptEngine::RegisterToEntity(scriptInstance, id);

						if (scriptInstance)
						{
							scriptInstance->OnAwake();
							scriptInstance->OnStart();
							scriptInstance->OnUpdate(aDeltaTime);
						}
						else
						{
							VT_CORE_WARN("Unable to create script with name {0} on entity {1}!", ScriptRegistry::GetNameFromGUID(scriptId), id);
						}
					}
				}
			});

		myRegistry.ForEach<MonoScriptComponent>([&](Wire::EntityId id, const MonoScriptComponent&)
			{
				MonoScriptEngine::OnUpdateEntityInstance(id, aDeltaTime);
			});

		myRegistry.ForEach<CameraComponent, TransformComponent>([this](Wire::EntityId, CameraComponent& cameraComp, const TransformComponent& transComp)
			{
				if (transComp.visible)
				{
					cameraComp.camera->SetPerspectiveProjection(cameraComp.fieldOfView, (float)myWidth / (float)myHeight, cameraComp.nearPlane, cameraComp.farPlane);
					cameraComp.camera->SetPosition(transComp.position);
					cameraComp.camera->SetRotation(gem::eulerAngles(transComp.rotation));
				}
			});


		myRegistry.ForEach<AnimationControllerComponent>([](Wire::EntityId, AnimationControllerComponent& animaitonTree)
			{
				animaitonTree.AnimTreeControl->Update();
			});

		myRegistry.ForEach<AnimatedCharacterComponent>([](Wire::EntityId, AnimatedCharacterComponent& animCharComp)
			{
				if (animCharComp.animatedCharacter != Asset::Null())
				{
					auto animChar = AssetManager::GetAsset<AnimatedCharacter>(animCharComp.animatedCharacter);
					if (animChar && animChar->IsValid())
					{
						if (animCharComp.isCrossFading)
						{
							if (animCharComp.crossfadeStartTime + animChar->GetCrossfadeTimeFromAnimations(animCharComp.crossfadeFrom, animCharComp.crossfadeTo) < AnimationManager::globalClock)
							{
								animCharComp.isCrossFading = false;
								animCharComp.shouldCrossfade = false;
								animCharComp.currentStartTime = AnimationManager::globalClock;
							}
						}
						else
						{
							if (animCharComp.shouldCrossfade)
							{
								animCharComp.isCrossFading = true;
								animCharComp.crossfadeStartTime = AnimationManager::globalClock;

							}
						}
					}

				}
			});

		myRegistry.ForEach<VideoPlayerComponent>([&](Wire::EntityId, VideoPlayerComponent& videoPlayer)
			{
				if (videoPlayer.lastVideoHandle != videoPlayer.videoHandle)
				{
					Ref<Video> video = AssetManager::GetAsset<Video>(videoPlayer.videoHandle);
					if (video && video->IsValid())
					{
						videoPlayer.videoHandle = video->handle;
						videoPlayer.lastVideoHandle = videoPlayer.videoHandle;

						video->Play(true);
					}
				}

				if (videoPlayer.videoHandle != Asset::Null())
				{
					Ref<Video> videoAsset = AssetManager::GetAsset<Video>(videoPlayer.videoHandle);
					if (videoAsset && videoAsset->IsValid())
					{
						videoAsset->Update(aDeltaTime);
					}
				}
			});

		myRegistry.ForEach<AnimationControllerComponent>([aDeltaTime](Wire::EntityId, AnimationControllerComponent& animaitonTree)
			{
				//Create Tree
				animaitonTree.AnimTreeControl->Update();
			});

		myParticleSystem->Update(aDeltaTime);

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

	void Scene::UpdateEditor(float aDeltaTime)
	{
		myStatistics.entityCount = (uint32_t)myRegistry.GetAllEntities().size();

		// Update scene data
		{
			SceneData sceneData;
			sceneData.deltaTime = aDeltaTime;
			sceneData.timeSinceStart = myTimeSinceStart;

			Renderer::SetSceneData(sceneData);
		}
	}

	void Scene::UpdateSimulation(float aDeltaTime)
	{
		Physics::GetScene()->Simulate(aDeltaTime);
		myPhysicsSystem->Update(aDeltaTime);

		myStatistics.entityCount = (uint32_t)myRegistry.GetAllEntities().size();

		// Update scene data
		{
			SceneData sceneData;
			sceneData.deltaTime = aDeltaTime;
			sceneData.timeSinceStart = myTimeSinceStart;

			Renderer::SetSceneData(sceneData);
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

		if (myIsPlaying && entity.HasComponent<ScriptComponent>())
		{
			auto& scriptComp = entity.GetComponent<ScriptComponent>();
			for (const auto& script : scriptComp.scripts)
			{
				ScriptEngine::UnregisterFromEntity(script, entity.GetId());
			}
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
					parentRelComp.Children.erase(it);
				}
			}

			for (int32_t i = (int32_t)relComp.Children.size() - 1; i >= 0; --i)
			{
				Entity childEnt{ relComp.Children.at(i), this };
				RemoveEntity(childEnt);
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

	gem::mat4 Scene::GetWorldSpaceTransform(Entity entity, bool accountForActor)
	{
		const auto& transformComp = entity.GetComponent<TransformComponent>();

		if (accountForActor && entity.HasComponent<RigidbodyComponent>())
		{
			//if (entity.GetComponent<RigidbodyComponent>().bodyType == BodyType::Dynamic)
			//{
			//	return transformComp.GetTransform();
			//}
		}

		gem::mat4 transform = { 1.0f };

		Entity parent = entity.GetParent();
		if (parent)
		{
			transform = GetWorldSpaceTransform(parent);
		}

		return transform * transformComp.GetTransform();
	}

	Scene::TQS Scene::GetWorldSpaceTRS(Entity entity)
	{
		TQS transform;

		gem::vec3 r;
		gem::decompose(GetWorldSpaceTransform(entity, false), transform.position, r, transform.scale);

		transform.rotation = gem::quat{ r };
		return transform;
	}

	Entity Scene::InstantiateSplitMesh(const std::filesystem::path& inPath)
	{
		auto mesh = AssetManager::GetAsset<Mesh>(inPath);
		if (!mesh || !mesh->IsValid())
		{
			VT_CORE_WARN("Trying to instantiate invalid mesh {0}!", inPath.string());
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
			meshComponent.subMeshIndex = i;
			i++;
		}

		return parentEntity;
	}

	gem::vec3 Scene::GetWorldForward(Entity entity)
	{
		gem::vec3 p, r, s;
		gem::decompose(GetWorldSpaceTransform(entity), p, r, s);

		const gem::quat orientation = gem::quat(r);
		return gem::rotate(orientation, gem::vec3{ 0.f, 0.f, 1.f });
	}

	gem::vec3 Scene::GetWorldRight(Entity entity)
	{
		gem::vec3 p, r, s;
		gem::decompose(GetWorldSpaceTransform(entity), p, r, s);

		const gem::quat orientation = gem::quat(r);
		return gem::rotate(orientation, gem::vec3{ 1.f, 0.f, 0.f });
	}

	gem::vec3 Scene::GetWorldUp(Entity entity)
	{
		gem::vec3 p, r, s;
		gem::decompose(GetWorldSpaceTransform(entity), p, r, s);

		const gem::quat orientation = gem::quat(r);
		return gem::rotate(orientation, gem::vec3{ 0.f, 1.f, 0.f });
	}

	const std::set<AssetHandle> Scene::GetDependencyList(const std::filesystem::path& scenePath)
	{
		const auto folderPath = scenePath.parent_path();
		const auto depPath = folderPath / "Dependencies.vtdep";

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
		if (!FileSystem::Exists(scenePath))
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
		if (!FileSystem::Exists(scenePath))
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

	void Scene::CopyTo(Ref<Scene> otherScene)
	{
		VT_PROFILE_FUNCTION();

		otherScene->myName = myName;
		otherScene->myEnvironment = myEnvironment;

		auto& otherRegistry = otherScene->GetRegistry();

		// Copy registry
		for (const auto& ent : myRegistry.GetAllEntities())
		{
			otherRegistry.AddEntity(ent);
			Entity::Copy(myRegistry, otherRegistry, ent, ent);
		}

		otherRegistry.ForEach<VisualScriptingComponent>([&](Wire::EntityId id, VisualScriptingComponent& comp) 
			{
				if (!comp.graph)
				{
					return;
				}

				auto& spec = comp.graph->GetSpecification();
				
				// Set entities to correct scene
				{
					for (const auto& n : spec.nodes)
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

		myRegistry.SetOnCreateFunction<BoxColliderComponent>([&](Wire::EntityId id, void* compPtr)
			{
				if (!myIsPlaying)
				{
					return;
				}

				auto entity = Entity{ id, this };
				auto actor = Physics::GetScene()->GetActor(entity);
				BoxColliderComponent& comp = *static_cast<BoxColliderComponent*>(compPtr);

				if (actor && !comp.added)
				{
					actor->AddCollider(comp, entity);
				}
			});

		myRegistry.SetOnCreateFunction<SphereColliderComponent>([&](Wire::EntityId id, void* compPtr)
			{
				if (!myIsPlaying)
				{
					return;
				}

				auto entity = Entity{ id, this };
				auto actor = Physics::GetScene()->GetActor(entity);
				SphereColliderComponent& comp = *static_cast<SphereColliderComponent*>(compPtr);

				if (actor && !comp.added)
				{
					actor->AddCollider(comp, entity);
				}
			});

		myRegistry.SetOnCreateFunction<CapsuleColliderComponent>([&](Wire::EntityId id, void* compPtr)
			{
				if (!myIsPlaying)
				{
					return;
				}

				auto entity = Entity{ id, this };
				auto actor = Physics::GetScene()->GetActor(entity);
				CapsuleColliderComponent& comp = *static_cast<CapsuleColliderComponent*>(compPtr);

				if (actor && !comp.added)
				{
					actor->AddCollider(comp, entity);
				}
			});

		myRegistry.SetOnCreateFunction<MeshColliderComponent>([&](Wire::EntityId id, void* compPtr)
			{
				if (!myIsPlaying)
				{
					return;
				}

				auto entity = Entity{ id, this };
				auto actor = Physics::GetScene()->GetActor(entity);
				MeshColliderComponent& comp = *static_cast<MeshColliderComponent*>(compPtr);

				if (actor && !comp.added)
				{
					actor->AddCollider(comp, entity);
				}
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

		const gem::mat4 transformMatrix = GetWorldSpaceTransform(entity);

		gem::vec3 r;
		gem::decompose(transformMatrix, transform.position, r, transform.scale);
	
		transform.rotation = gem::quat{ r };
	}

	void Scene::ConvertToLocalSpace(Entity entity)
	{
		Entity parent = entity.GetParent();

		if (!parent)
		{
			return;
		}

		auto& transform = entity.GetComponent<TransformComponent>();
		const gem::mat4 parentTransform = GetWorldSpaceTransform(parent);
		const gem::mat4 localTransform = gem::inverse(parentTransform) * transform.GetTransform();

		gem::vec3 r;
		gem::decompose(localTransform, transform.position, r, transform.scale);
		transform.rotation = gem::quat{ r };
	}

	void Scene::SortScene()
	{
		myRegistry.Sort([](const auto& lhs, const auto& rhs)
			{
				return lhs < rhs;
			});
	}
}