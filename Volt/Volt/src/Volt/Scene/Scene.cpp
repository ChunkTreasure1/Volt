#include "vtpch.h"
#include "Scene.h"

#include <Volt/Core/Application.h>

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Video/Video.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Core/Profiling.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/AudioComponents.h"
#include "Volt/Components/LightComponents.h"
#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Animation/AnimationManager.h"
#include "Volt/Animation/AnimationController.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoGCManager.h"

#include "Volt/Utility/FileSystem.h"

#include "Volt/Math/Math.h"

#include "Volt/RenderingNew/RenderScene.h"

#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"

#include "Volt/Vision/Vision.h"
#include "Volt/Utility/Random.h"

#include "Volt/Discord/DiscordSDK.h"

#include <GraphKey/TimerManager.h>
#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>

#include <Navigation/Core/NavigationSystem.h>

#include <stack>
#include <ranges>

namespace Volt
{
	Scene::Scene(const std::string& name)
		: m_name(name), m_animationSystem(this)
	{
		m_visionSystem = CreateRef<Vision>(this);
		m_renderScene = CreateRef<RenderScene>(this);

		SetupComponentFunctions();
		AddLayer("Main", 0);

		m_worldEngine.Reset(this, 16, 4);
	}

	void Scene::PostInitialize()
	{
	}

	Scene::~Scene()
	{
	}

	Scene::Scene()
		: m_animationSystem(this)
	{
		m_visionSystem = CreateRef<Vision>(this);
		m_renderScene = CreateRef<RenderScene>(this);

		SetupComponentFunctions();
		AddLayer("Main", 0);

		m_worldEngine.Reset(this, 16, 4);
	}

	void Scene::OnEvent(Event& e)
	{
		VT_PROFILE_SCOPE((std::string("Scene::OnEvent: ") + std::string(e.GetName())).c_str());

		if (!m_isPlaying)
		{
			return;
		}

		ForEachWithComponents<const AnimationControllerComponent>([&](const entt::entity id, const AnimationControllerComponent& controller)
		{
			if (controller.controller)
			{
				controller.controller->GetGraph()->OnEvent(e);
			}
		});

		m_audioSystem.OnEvent(m_registry, e);
		m_visionSystem->OnEvent(e);
	}

	void Scene::SetRenderSize(uint32_t aWidth, uint32_t aHeight)
	{
		m_viewportWidth = aWidth;
		m_viewportHeight = aHeight;
	}

	void Scene::SetTimeScale(const float aTimeScale)
	{
		Volt::Application::Get().SetTimeScale(aTimeScale);
	}

	void Scene::InitializeEngineScripts()
	{
		MonoScriptEngine::OnRuntimeStart(this);

		// Awake
		{
			ForEachWithComponents<const MonoScriptComponent, const IDComponent>([](const entt::entity id, const MonoScriptComponent& scriptComp, const IDComponent& idComponent)
			{
				for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
				{
					auto scriptClass = MonoScriptEngine::GetScriptClass(scriptComp.scriptNames[i]);

					if (scriptClass && scriptClass->IsEngineScript())
					{
						MonoScriptEngine::OnAwakeInstance(scriptComp.scriptIds[i], idComponent.id, scriptComp.scriptNames[i]);
					}
				}
			});
		}

		MonoScriptEngine::DoOnAwakeInstance();

		// Create
		{
			ForEachWithComponents<const MonoScriptComponent, const IDComponent>([&](entt::entity id, const MonoScriptComponent& scriptComp, const IDComponent& idComponent)
			{
				for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
				{
					auto scriptClass = MonoScriptEngine::GetScriptClass(scriptComp.scriptNames[i]);

					if (scriptClass && scriptClass->IsEngineScript())
					{
						MonoScriptEngine::OnCreateInstance(scriptComp.scriptIds[i], idComponent.id, scriptComp.scriptNames[i]);
					}
				}
			});
		}
	}

	void Scene::ShutdownEngineScripts()
	{
		ForEachWithComponents<const MonoScriptComponent>([&](entt::entity id, const MonoScriptComponent& scriptComp)
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

		m_isPlaying = true;
		m_timeSinceStart = 0.f;

		m_audioSystem.RuntimeStart(m_registry, shared_from_this());

		Application::Get().GetNavigationSystem().OnRuntimeStart();

		MonoScriptEngine::OnRuntimeStart(this);

		ForEachWithComponents<CommonComponent>([](entt::entity, CommonComponent& dataComp)
		{
			dataComp.timeSinceCreation = 0.f;
			dataComp.randomValue = Random::Float(0.f, 1.f);
		});

		m_visionSystem->Initialize();
		m_animationSystem.OnRuntimeStart(m_registry);

		ForEachWithComponents<const MonoScriptComponent, const IDComponent>([&](entt::entity id, const MonoScriptComponent& scriptComp, const IDComponent& idComponent)
		{
			for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				MonoScriptEngine::OnAwakeInstance(scriptComp.scriptIds[i], idComponent.id, scriptComp.scriptNames[i]);
			}
		});

		MonoScriptEngine::DoOnAwakeInstance();

		ForEachWithComponents<const MonoScriptComponent, const IDComponent>([&](entt::entity id, const MonoScriptComponent& scriptComp, const IDComponent& idComponent)
		{
			for (uint32_t i = 0; i < scriptComp.scriptIds.size(); i++)
			{
				MonoScriptEngine::OnCreateInstance(scriptComp.scriptIds[i], idComponent.id, scriptComp.scriptNames[i]);
			}
		});
	}

	void Scene::OnRuntimeEnd()
	{
		m_isPlaying = false;
		GraphKey::TimerManager::Clear();

		ForEachWithComponents<const MonoScriptComponent>([&](entt::entity id, const MonoScriptComponent& scriptComp)
		{
			for (const auto& instId : scriptComp.scriptIds)
			{
				MonoScriptEngine::OnDestroyInstance(instId);
			}
		});

		MonoScriptEngine::OnRuntimeEnd();

		m_animationSystem.OnRuntimeEnd(m_registry);
		Physics::DestroyScene();
		m_audioSystem.RuntimeStop(m_registry, shared_from_this());
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
		m_statistics.entityCount = static_cast<uint32_t>(m_registry.alive());

		AnimationManager::Update(aDeltaTime);
		Physics::GetScene()->Simulate(aDeltaTime);
		m_visionSystem->Update(aDeltaTime);

		m_timeSinceStart += aDeltaTime;
		m_currentDeltaTime = aDeltaTime;

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

			ForEachWithComponents<CommonComponent>([&](entt::entity id, CommonComponent& dataComp)
			{
				dataComp.timeSinceCreation += aDeltaTime;
			});
		}

		MonoScriptEngine::DoDestroyQueue();
		MonoScriptEngine::OnUpdate(aDeltaTime);

		ForEachWithComponents<const MonoScriptComponent, const TransformComponent>([&](entt::entity id, const MonoScriptComponent& scriptComp, const TransformComponent& transformComponent)
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

		ForEachWithComponents<CameraComponent, const TransformComponent>([&](entt::entity id, CameraComponent& cameraComp, const TransformComponent& transComp)
		{
			if (transComp.visible)
			{
				Entity entity = { id, this };

				cameraComp.camera->SetPerspectiveProjection(cameraComp.fieldOfView, (float)m_viewportWidth / (float)m_viewportHeight, cameraComp.nearPlane, cameraComp.farPlane);
				cameraComp.camera->SetPosition(entity.GetPosition());
				cameraComp.camera->SetRotation(glm::eulerAngles(entity.GetRotation()));
			}
		});

		m_particleSystem.Update(m_registry, shared_from_this(), aDeltaTime);
		m_audioSystem.Update(m_registry, shared_from_this(), aDeltaTime);
		m_animationSystem.Update(m_registry, aDeltaTime);
	}

	void Scene::FixedUpdate(float aDeltaTime)
	{
	}

	void Scene::UpdateEditor(float aDeltaTime)
	{
		VT_PROFILE_FUNCTION();

		m_statistics.entityCount = static_cast<uint32_t>(m_registry.size());
		m_particleSystem.Update(m_registry, shared_from_this(), aDeltaTime);

		// Update scene data
		{
			//SceneData sceneData;
			//sceneData.deltaTime = aDeltaTime;
			//sceneData.timeSinceStart = m_timeSinceStart;

			//Renderer::SetSceneData(sceneData);
		}

		ForEachWithComponents<CameraComponent, const TransformComponent>([&](entt::entity id, CameraComponent& cameraComp, const TransformComponent& transComp)
		{
			if (transComp.visible)
			{
				Entity entity = { id, this };

				if (!cameraComp.camera)
				{
					return;
				}

				cameraComp.camera->SetPerspectiveProjection(cameraComp.fieldOfView, (float)m_viewportWidth / (float)m_viewportHeight, cameraComp.nearPlane, cameraComp.farPlane);
				cameraComp.camera->SetPosition(entity.GetPosition());
				cameraComp.camera->SetRotation(glm::eulerAngles(entity.GetRotation()));
			}
		});
	}

	void Scene::UpdateSimulation(float aDeltaTime)
	{
		Physics::GetScene()->Simulate(aDeltaTime);

		m_statistics.entityCount = static_cast<uint32_t>(m_registry.alive());
	}

	Entity Scene::CreateEntity(const std::string& tag)
	{
		entt::entity id = m_registry.create();

		Entity newEntity = Entity(id, this);
		auto& transform = newEntity.AddComponent<TransformComponent>();
		transform.position = { 0.f, 0.f, 0.f };
		transform.rotation = { 1.f, 0.f, 0.f, 0.f };
		transform.scale = { 1.f, 1.f, 1.f };

		// Tag
		{
			auto& tagComp = newEntity.AddComponent<TagComponent>();
			if (tag.empty())
			{
				tagComp.tag = "New Entity";
			}
			else
			{
				tagComp.tag = tag;
			}
		}
		newEntity.AddComponent<CommonComponent>();
		newEntity.AddComponent<RelationshipComponent>();
		
		auto& idComp = newEntity.AddComponent<IDComponent>();

		while (m_entityRegistry.Contains(idComp.id))
		{
			idComp.id = {};
		}

		newEntity.GetComponent<CommonComponent>().layerId = m_sceneLayers.at(m_activeLayerIndex).id;
		newEntity.GetComponent<CommonComponent>().randomValue = Random::Float(0.f, 1.f);
		newEntity.GetComponent<CommonComponent>().timeSinceCreation = 0.f;

		const auto uuid = newEntity.GetComponent<IDComponent>().id;

		m_entityRegistry.AddEntity(newEntity);
		m_entityRegistry.MarkEntityAsEdited(newEntity);
		m_worldEngine.AddEntity(newEntity);

		InvalidateEntityTransform(uuid);
		SortScene();
		return newEntity;
	}

	Entity Scene::CreateEntityWithUUID(const EntityID& uuid, const std::string& tag)
	{
		entt::entity id = m_registry.create();

		Entity newEntity = Entity(id, this);
		auto& transform = newEntity.AddComponent<TransformComponent>();
		transform.position = { 0.f, 0.f, 0.f };
		transform.rotation = { 1.f, 0.f, 0.f, 0.f };
		transform.scale = { 1.f, 1.f, 1.f };

		// Tag
		{
			auto& tagComp = newEntity.AddComponent<TagComponent>();
			if (tag.empty())
			{
				tagComp.tag = "New Entity";
			}
			else
			{
				tagComp.tag = tag;
			}
		}
		newEntity.AddComponent<CommonComponent>();
		newEntity.AddComponent<RelationshipComponent>();
		newEntity.AddComponent<IDComponent>();

		newEntity.GetComponent<CommonComponent>().layerId = m_sceneLayers.at(m_activeLayerIndex).id;
		newEntity.GetComponent<CommonComponent>().randomValue = Random::Float(0.f, 1.f);
		newEntity.GetComponent<CommonComponent>().timeSinceCreation = 0.f;

		newEntity.GetComponent<IDComponent>().id = uuid;

		m_entityRegistry.AddEntity(newEntity);
		m_worldEngine.AddEntity(newEntity);

		InvalidateEntityTransform(uuid);
		SortScene();

		return newEntity;
	}

	Entity Scene::GetEntityFromUUID(const EntityID uuid) const
	{
		if (!m_entityRegistry.Contains(uuid))
		{
			return Entity::Null();
		}

		return { m_entityRegistry.GetHandleFromUUID(uuid), const_cast<Scene*>(this) }; // #TODO_Ivar: Doubtable
	}

	entt::entity Scene::GetHandleFromUUID(const EntityID uuid) const
	{
		return m_entityRegistry.GetHandleFromUUID(uuid);
	}

	void Scene::RemoveEntity(Entity entity)
	{
		RemoveEntityInternal(entity, false);
		SortScene();
	}

	void Scene::ParentEntity(Entity parent, Entity child)
	{
		if (!parent.IsValid() || !child.IsValid() || parent == child)
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

		auto& childChildren = child.GetComponent<RelationshipComponent>().children;

		if (auto it = std::find(childChildren.begin(), childChildren.end(), parent.GetID()) != childChildren.end())
		{
			return;
		}

		child.GetComponent<RelationshipComponent>().parent = parent.GetID();
		parent.GetComponent<RelationshipComponent>().children.emplace_back(child.GetID());

		if (child.GetLayerID() != parent.GetLayerID())
		{
			MoveToLayer(child, parent.GetLayerID());
		}

		ConvertToLocalSpace(child);
	}

	void Scene::UnparentEntity(Entity entity)
	{
		if (!entity.IsValid()) { return; }

		auto parent = entity.GetParent();
		if (!parent.IsValid())
		{
			return;
		}

		auto& children = parent.GetComponent<RelationshipComponent>().children;

		auto it = std::find(children.begin(), children.end(), entity.GetID());
		if (it != children.end())
		{
			children.erase(it);
		}

		ConvertToWorldSpace(entity);
		entity.GetComponent<RelationshipComponent>().parent = Entity::NullID();
	}

	void Scene::InvalidateEntityTransform(const EntityID& entityUUID)
	{
		std::vector<EntityID> entityStack;
		entityStack.reserve(10);
		entityStack.push_back(entityUUID);

		while (!entityStack.empty())
		{
			const EntityID currentUUID = entityStack.back();
			entityStack.pop_back();

			Volt::Entity ent{ m_entityRegistry.GetHandleFromUUID(currentUUID), this };

			if (!ent.HasComponent<RelationshipComponent>())
			{
				continue;
			}

			auto& relComp = ent.GetComponent<RelationshipComponent>();

			for (const auto& child : relComp.children)
			{
				entityStack.push_back(child);
			}

			{
				std::unique_lock lock{ m_cachedEntityTransformMutex };
				if (m_cachedEntityTransforms.contains(currentUUID))
				{
					m_cachedEntityTransforms.erase(currentUUID);
				}
			}

			if (m_sceneSettings.useWorldEngine)
			{
				m_worldEngine.OnEntityMoved(ent);
			}
		}

		m_renderScene->Invalidate();
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

	const Entity Scene::GetEntityWithName(std::string name)
	{
		Entity entity;

		auto view = m_registry.view<TagComponent>();
		for (const auto& id : view)
		{
			if (m_registry.get<TagComponent>(id).tag == name)
			{
				return Entity{ id, this };
			}
		}

		return entity;
	}

	const bool Scene::IsEntityValid(EntityID entityId) const
	{
		return m_registry.valid(m_entityRegistry.GetHandleFromUUID(entityId));
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
			if (AssetManager::IsLoaded(dep))
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
				// #TODO_Ivar: Readd
				//auto ent = newScene->CreateEntity("Cube");

				//auto id = ent.GetComponent<IDComponent>();

				//auto& meshComp = ent.AddComponent<MeshComponent>();
				//meshComp.handle = AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cube.vtmesh");

				//ent.AddComponent<RigidbodyComponent>();
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

				ent.SetPosition({ 0.f, 0.f, -5.f });
			}
		}

		newScene->InvalidateRenderScene();
		return newScene;
	}

	void Scene::CopyTo(Ref<Scene> otherScene)
	{
		VT_PROFILE_FUNCTION();

		otherScene->m_name = m_name;
		otherScene->m_environment = m_environment;
		otherScene->handle = handle;
		otherScene->m_sceneLayers = m_sceneLayers;
		otherScene->m_activeLayerIndex = m_activeLayerIndex;
		otherScene->m_lastLayerId = m_lastLayerId;

		m_registry.each([&](entt::entity id)
		{
			const EntityID uuid = m_registry.get<IDComponent>(id).id;

			auto entity =  otherScene->CreateEntityWithUUID(uuid);
			Entity::Copy(Entity{ id, this }, entity, EntityCopyFlags::None);
		});
	}

	void Scene::Clear()
	{
		m_registry.clear();
	}

	const glm::mat4 Scene::GetWorldTransform(Entity entity) const
	{
		{
			std::shared_lock lock(m_cachedEntityTransformMutex);
			if (m_cachedEntityTransforms.contains(entity.GetID()))
			{
				return m_cachedEntityTransforms.at(entity.GetID());
			}
		}

		const auto tqs = GetWorldTQS(entity);
		const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, tqs.position) * glm::mat4_cast(tqs.rotation) * glm::scale(glm::mat4{ 1.f }, tqs.scale);

		{
			std::unique_lock lock{ m_cachedEntityTransformMutex };
			m_cachedEntityTransforms[entity.GetID()] = transform;
		}

		return transform;
	}

	const std::vector<Entity> Scene::FlattenEntityHeirarchy(Entity entity)
	{
		std::vector<Entity> result;
		result.emplace_back(entity);

		for (auto child : entity.GetChildren())
		{
			auto childResult = FlattenEntityHeirarchy(child);

			for (auto& childRes : childResult)
			{
				result.emplace_back(childRes);
			}
		}

		return result;
	}

	const Scene::TQS Scene::GetWorldTQS(Entity entity) const
	{
		std::vector<Entity> hierarchy{};
		hierarchy.emplace_back(entity);

		Entity currentEntity = entity;
		while (currentEntity.HasParent())
		{
			auto parent = currentEntity.GetParent();
			hierarchy.emplace_back(parent);
			currentEntity = parent;
		}

		TQS resultTransform{};
		for (const auto& ent : std::ranges::reverse_view(hierarchy))
		{
			const auto& transComp = m_registry.get<TransformComponent>((entt::entity)ent);

			resultTransform.position = resultTransform.position + resultTransform.rotation * transComp.position;
			resultTransform.rotation = resultTransform.rotation * transComp.rotation;
			resultTransform.scale = resultTransform.scale * transComp.scale;
		}

		return resultTransform;
	}

	void Scene::MoveToLayerRecursive(Entity entity, uint32_t targetLayer)
	{
		if (!entity.HasComponent<CommonComponent>())
		{
			return;
		}

		entity.GetComponent<CommonComponent>().layerId = targetLayer;

		for (const auto& child : entity.GetChildren())
		{
			MoveToLayerRecursive(child, targetLayer);
		}
	}

	void Scene::SetupComponentFunctions()
	{
		m_registry.on_construct<RigidbodyComponent>().connect<&Scene::RigidbodyComponent_OnCreate>(this);
		m_registry.on_construct<CharacterControllerComponent>().connect<&Scene::CharacterControllerComponent_OnCreate>(this);
		m_registry.on_construct<BoxColliderComponent>().connect<&Scene::BoxColliderComponent_OnCreate>(this);
		m_registry.on_construct<SphereColliderComponent>().connect<&Scene::SphereColliderComponent_OnCreate>(this);
		m_registry.on_construct<CapsuleColliderComponent>().connect<&Scene::CapsuleColliderComponent_OnCreate>(this);
		m_registry.on_construct<MeshColliderComponent>().connect<&Scene::MeshColliderComponent_OnCreate>(this);
		m_registry.on_construct<AudioSourceComponent>().connect<&Scene::AudioSourceComponent_OnCreate>(this);
		m_registry.on_construct<AudioListenerComponent>().connect<&Scene::AudioListenerComponent_OnCreate>(this);
		m_registry.on_construct<CameraComponent>().connect<&Scene::CameraComponent_OnCreate>(this);

		m_registry.on_destroy<RigidbodyComponent>().connect<&Scene::RigidbodyComponent_OnDestroy>(this);
		m_registry.on_destroy<CharacterControllerComponent>().connect<&Scene::CharacterControllerComponent_OnDestroy>(this);
		m_registry.on_destroy<BoxColliderComponent>().connect<&Scene::BoxColliderComponent_OnDestroy>(this);
		m_registry.on_destroy<SphereColliderComponent>().connect<&Scene::SphereColliderComponent_OnDestroy>(this);
		m_registry.on_destroy<CapsuleColliderComponent>().connect<&Scene::CapsuleColliderComponent_OnDestroy>(this);
		m_registry.on_destroy<MeshColliderComponent>().connect<&Scene::MeshColliderComponent_OnDestroy>(this);
		m_registry.on_destroy<MeshComponent>().connect<&Scene::MeshComponent_OnDestroy>(this);
	}

	const bool Scene::IsRelatedTo(Entity entity, Entity otherEntity)
	{
		const auto flatHeirarchy = FlattenEntityHeirarchy(entity);
		for (const auto& ent : flatHeirarchy)
		{
			if (ent.GetID() == otherEntity.GetID())
			{
				return true;
			}
		}

		return false;
	}

	void Scene::IsRecursiveChildOf(Entity parent, Entity currentEntity, bool& outChild)
	{
		if (currentEntity.HasComponent<RelationshipComponent>())
		{
			auto& relComp = currentEntity.GetComponent<RelationshipComponent>();
			for (const auto& childId : relComp.children)
			{
				Entity child{ m_entityRegistry.GetHandleFromUUID(childId), this };

				outChild |= (parent.GetID() == childId) && (childId != parent.GetID());

				IsRecursiveChildOf(parent, child, outChild);
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

		const glm::mat4 transformMatrix = GetWorldTransform(entity);

		glm::vec3 r;
		Math::Decompose(transformMatrix, transform.position, r, transform.scale);

		transform.rotation = glm::quat{ r };

		InvalidateEntityTransform(entity.GetID());
	}

	void Scene::ConvertToLocalSpace(Entity entity)
	{
		Entity parent = entity.GetParent();

		if (!parent)
		{
			return;
		}

		auto& transform = entity.GetComponent<TransformComponent>();
		const glm::mat4 parentTransform = GetWorldTransform(parent);
		const glm::mat4 localTransform = glm::inverse(parentTransform) * transform.GetTransform();

		glm::vec3 r;
		Math::Decompose(localTransform, transform.position, r, transform.scale);
		transform.rotation = glm::quat{ r };

		InvalidateEntityTransform(entity.GetID());
	}

	void Scene::RemoveEntityInternal(Entity entity, bool removingParent)
	{
		if (!m_registry.valid(entity))
		{
			return;
		}

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& relComp = entity.GetComponent<RelationshipComponent>();
			if (relComp.parent != Entity::NullID())
			{
				Entity parentEnt = { m_entityRegistry.GetHandleFromUUID(relComp.parent), this };

				if (parentEnt.HasComponent<RelationshipComponent>() && !removingParent)
				{
					auto& parentRelComp = parentEnt.GetComponent<RelationshipComponent>();

					auto it = std::find(parentRelComp.children.begin(), parentRelComp.children.end(), entity.GetID());
					if (it != parentRelComp.children.end())
					{
						parentRelComp.children.erase(it);
					}
				}
			}

			for (int32_t i = static_cast<int32_t>(relComp.children.size()) - 1; i >= 0; --i)
			{
				Entity childEnt{ m_entityRegistry.GetHandleFromUUID(relComp.children.at(i)), this };
				RemoveEntityInternal(childEnt, true);
			
				relComp = entity.GetComponent<RelationshipComponent>();
			}
		}

		if (entity.HasComponent<MonoScriptComponent>())
		{
			for (const auto& scriptId : entity.GetComponent<MonoScriptComponent>().scriptIds)
			{
				MonoScriptEngine::OnDestroyInstance(scriptId);
			}
		}

		m_entityRegistry.RemoveEntity(entity);
		m_registry.destroy(entity);
	}

	void Scene::AddLayer(const std::string& layerName, uint32_t layerId)
	{
		m_sceneLayers.emplace_back(layerId, layerName);
	}

	void Scene::SetLayers(const std::vector<SceneLayer>& sceneLayers)
	{
		m_sceneLayers = sceneLayers;

		for (const auto& layer : m_sceneLayers)
		{
			m_lastLayerId = std::max(m_lastLayerId, layer.id);
		}
	}

	void Scene::RigidbodyComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		Physics::CreateActor(Entity{ id, this });
	}

	void Scene::CharacterControllerComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		Physics::CreateControllerActor(Entity{ id, this });
	}

	void Scene::BoxColliderComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		auto entity = Entity{ id, this };

		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(entity);
		BoxColliderComponent& comp = registry.get<BoxColliderComponent>(id);

		if (actor && !comp.added)
		{
			actor->AddCollider(comp, entity);
		}
	}

	void Scene::SphereColliderComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		auto entity = Entity{ id, this };

		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(entity);
		SphereColliderComponent& comp = registry.get<SphereColliderComponent>(id);

		if (actor && !comp.added)
		{
			actor->AddCollider(comp, entity);
		}
	}

	void Scene::CapsuleColliderComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		auto entity = Entity{ id, this };

		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(entity);
		CapsuleColliderComponent& comp = registry.get<CapsuleColliderComponent>(id);

		if (actor && !comp.added)
		{
			actor->AddCollider(comp, entity);
		}
	}

	void Scene::MeshColliderComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		auto entity = Entity{ id, this };

		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(entity);
		MeshColliderComponent& comp = registry.get<MeshColliderComponent>(id);

		if (actor && !comp.added)
		{
			actor->AddCollider(comp, entity);
		}
	}

	void Scene::AudioSourceComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		AudioSourceComponent& comp = registry.get<AudioSourceComponent>(id);
		comp.OnCreate(id);
	}

	void Scene::AudioListenerComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		AudioListenerComponent& comp = registry.get<AudioListenerComponent>(id);
		comp.OnCreate(id);
	}

	void Scene::CameraComponent_OnCreate(entt::registry& registry, entt::entity id)
	{
		CameraComponent& camComp = registry.get<CameraComponent>(id);
		camComp.camera = CreateRef<Camera>(camComp.fieldOfView, 1.f, float(m_viewportWidth) / float(m_viewportHeight), camComp.nearPlane, camComp.farPlane);
	}

	void Scene::RigidbodyComponent_OnDestroy(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
		if (actor)
		{
			Physics::GetScene()->RemoveActor(actor);
		}
	}

	void Scene::CharacterControllerComponent_OnDestroy(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetControllerActor(Entity{ id, this });
		if (actor)
		{
			Physics::GetScene()->RemoveControllerActor(actor);
		}
	}

	void Scene::BoxColliderComponent_OnDestroy(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
		if (actor)
		{
			actor->RemoveCollider(ColliderType::Box);
		}
	}

	void Scene::SphereColliderComponent_OnDestroy(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
		if (actor)
		{
			actor->RemoveCollider(ColliderType::Sphere);
		}
	}

	void Scene::CapsuleColliderComponent_OnDestroy(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
		if (actor)
		{
			actor->RemoveCollider(ColliderType::Capsule);
		}
	}

	void Scene::MeshColliderComponent_OnDestroy(entt::registry& registry, entt::entity id)
	{
		if (!m_isPlaying)
		{
			return;
		}

		auto actor = Physics::GetScene()->GetActor(Entity{ id, this });
		if (actor)
		{
			auto& comp = registry.get<MeshColliderComponent>(id);
			if (comp.isConvex)
			{
				actor->RemoveCollider(ColliderType::ConvexMesh);
			}
			else
			{
				actor->RemoveCollider(ColliderType::TriangleMesh);
			}
		}
	}

	void Scene::MeshComponent_OnDestroy(entt::registry& registry, entt::entity id)
	{
		auto& meshComp = registry.get<MeshComponent>(id);
		for (const auto& renderId : meshComp.renderObjectIds)
		{
			m_renderScene->Unregister(renderId);
		}
	}

	void Scene::SortScene()
	{
		m_registry.sort<CommonComponent>([](const entt::entity lhs, const entt::entity rhs)
		{
			return lhs < rhs;
		});
	}

	void Scene::AddLayer(const std::string& layerName)
	{
		m_sceneLayers.emplace_back(m_lastLayerId++, layerName);
	}

	void Scene::RemoveLayer(const std::string& layerName)
	{
		m_sceneLayers.erase(std::remove_if(m_sceneLayers.begin(), m_sceneLayers.end(), [&](const SceneLayer& lhs)
		{
			return lhs.name == layerName;
		}), m_sceneLayers.end());
	}

	void Scene::RemoveLayer(uint32_t layerId)
	{
		m_sceneLayers.erase(std::remove_if(m_sceneLayers.begin(), m_sceneLayers.end(), [&](const SceneLayer& lhs)
		{
			return lhs.id == layerId;
		}), m_sceneLayers.end());
	}

	void Scene::MoveToLayer(Entity entity, uint32_t targetLayer)
	{
		if (entity.GetParent())
		{
			if (entity.GetParent().GetLayerID() != targetLayer)
			{
				entity.ClearParent();
			}
		}

		MoveToLayerRecursive(entity, targetLayer);
	}

	void Scene::SetActiveLayer(uint32_t layerId)
	{
		auto it = std::find_if(m_sceneLayers.begin(), m_sceneLayers.end(), [&](const auto& lhs) { return lhs.id == layerId; });

		if (it != m_sceneLayers.end())
		{
			m_activeLayerIndex = (uint32_t)std::distance(m_sceneLayers.begin(), it);
		}

		auto& act = Volt::DiscordSDK::GetRichPresence();

		act.GetParty().GetSize().SetCurrentSize(GetActiveLayer() + 1);
		act.GetParty().GetSize().SetMaxSize(static_cast<int32_t>(GetLayers().size()));

		Volt::DiscordSDK::UpdateRichPresence();
	}

	bool Scene::LayerExists(uint32_t layerId)
	{
		return std::find_if(m_sceneLayers.begin(), m_sceneLayers.end(), [layerId](const auto& lhs) { return lhs.id == layerId; }) != m_sceneLayers.end();
	}

	void Scene::MarkEntityAsEdited(const Entity& entity)
	{
		m_entityRegistry.MarkEntityAsEdited(entity);
	}

	void Scene::ClearEditedEntities()
	{
		m_entityRegistry.ClearEditedEntities();
	}

	const std::vector<Entity> Scene::GetAllEntities() const
	{
		std::vector<Entity> result{};
		result.reserve(m_registry.alive());

		m_registry.each([&](const entt::entity id)
		{
			result.emplace_back(Entity{ id, const_cast<Scene*>(this) }); // Doubtable
		});

		return result;
	}

	const std::vector<Entity> Scene::GetAllEditedEntities() const
	{
		std::vector<Entity> entities;

		for (const auto& entity : m_entityRegistry.GetEditedEntities())
		{
			entities.push_back(GetEntityFromUUID(entity));
		}

		return entities;
	}

	const std::vector<EntityID> Scene::GetAllRemovedEntities() const
	{
		std::vector<EntityID> entities;

		for (const auto& entity : m_entityRegistry.GetRemovedEntities())
		{
			entities.push_back(entity);
		}

		return entities;
	}
	
	void Scene::InvalidateRenderScene()
	{
		const auto& meshView = m_registry.view<MeshComponent, IDComponent>();
		for (const auto& id : meshView)
		{
			auto& meshComp = m_registry.get<MeshComponent>(id);
			auto& idComp = m_registry.get<IDComponent>(id);

			for (const auto& uuid : meshComp.renderObjectIds)
			{
				m_renderScene->Unregister(uuid);
			}

			meshComp.renderObjectIds.clear();

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComp.handle);
			if (!mesh || !mesh->IsValid())
			{
				continue;
			}

			const auto& materialTable = mesh->GetMaterialTable();

			for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
			{
				const auto materialIndex = mesh->GetSubMeshes().at(i).materialIndex;

				Ref<Material> mat = materialTable.GetMaterial(materialIndex);																							 
				if (meshComp.materials.at(materialIndex) != mat->handle)
				{
					Ref<Material> tempMat = AssetManager::GetAsset<Material>(meshComp.materials.at(materialIndex));
					if (tempMat && tempMat->IsValid())
					{
						mat = tempMat;
					}
				}

				auto uuid = m_renderScene->Register(idComp.id, mesh, mat, static_cast<uint32_t>(i));
				meshComp.renderObjectIds.emplace_back(uuid);
			}
		}
	}
}
