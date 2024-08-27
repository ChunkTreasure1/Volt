#include "vtpch.h"
#include "Scene.h"

#include "Volt/Core/Application.h"

#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Scene/Entity.h"

#include "Volt/Components/AudioComponents.h"
#include "Volt/Components/LightComponents.h"
#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Animation/AnimationManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

#include "Volt/Utility/FileSystem.h"

#include "Volt/Math/Math.h"

#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Vision/Vision.h"
#include "Volt/Utility/Random.h"

#include "Volt/Animation/MotionWeaver.h"

#include "Volt/Project/ProjectManager.h"

#include <AssetSystem/AssetManager.h>
#include <Navigation/Core/NavigationSystem.h>
#include <EntitySystem/Scripting/ECSSystemRegistry.h>

#include <CoreUtilities/Time/TimeUtility.h>
#include <CoreUtilities/FileSystem.h>

#include <stack>
#include <ranges>

namespace Volt
{
	VT_REGISTER_ASSET_FACTORY(AssetTypes::Scene, Scene);

	Scene::Scene(const std::string& name)
		: m_name(name)
	{
		Initialize();
	}

	Scene::Scene()
	{
		Initialize();
	}

	Scene::~Scene()
	{
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

	void Scene::OnRuntimeStart()
	{
		Physics::CreateScene(this);
		Physics::CreateActors(this);
		AnimationManager::Reset();

		m_isPlaying = true;
		m_timeSinceStart = 0.f;

		m_audioSystem.RuntimeStart(m_registry, shared_from_this());

		Application::Get().GetNavigationSystem().OnRuntimeStart();

		ForEachWithComponents<CommonComponent>([](entt::entity, CommonComponent& dataComp)
		{
			dataComp.timeSinceCreation = 0.f;
			dataComp.randomValue = Random::Float(0.f, 1.f);
		});

		m_visionSystem->Initialize();

		ForEachWithComponents<MotionWeaveComponent, const MeshComponent>([&](entt::entity id, MotionWeaveComponent& motionWeave, const MeshComponent& meshComp)
		{
			motionWeave.MotionWeaver = MotionWeaver::Create(motionWeave.motionWeaveDatabase);

			Entity entity{ id, shared_from_this() };

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComp.handle);
			if (mesh && mesh->IsValid())
			{
				const auto& idComp = entity.GetComponent<IDComponent>();
				const auto& materialTable = mesh->GetMaterialTable();

				for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
				{
					auto material = AssetManager::QueueAsset<Material>(materialTable.GetMaterial(mesh->GetSubMeshes().at(i).materialIndex));
					if (!material->IsValid())
					{
					}

					auto uuid = m_renderScene->Register(idComp.id, motionWeave.MotionWeaver, mesh, material, static_cast<uint32_t>(i));
					motionWeave.renderObjectIds.emplace_back(uuid);
				}
			}
		});
	}

	void Scene::OnRuntimeEnd()
	{
		m_isPlaying = false;

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

		{
			VT_PROFILE_SCOPE("Update entity time");

			ForEachWithComponents<CommonComponent>([&](entt::entity id, CommonComponent& dataComp)
			{
				dataComp.timeSinceCreation += aDeltaTime;
			});
		}

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

		ForEachWithComponents<MotionWeaveComponent, const MeshComponent>([&](entt::entity id, MotionWeaveComponent& motionWeave, const MeshComponent& mesh)
		{
			if (motionWeave.MotionWeaver)
			{
				motionWeave.MotionWeaver->Update(aDeltaTime);
			}
		});

		m_particleSystem.Update(m_registry, shared_from_this(), aDeltaTime);
		m_audioSystem.Update(m_registry, shared_from_this(), aDeltaTime);
	}

	void Scene::FixedUpdate(float aDeltaTime)
	{
	}

	void Scene::UpdateEditor(float aDeltaTime)
	{
		VT_PROFILE_FUNCTION();

		m_statistics.entityCount = static_cast<uint32_t>(m_registry.size());
		m_particleSystem.Update(m_registry, shared_from_this(), aDeltaTime);

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
		newEntity.GetComponent<CommonComponent>().timeCreatedID = TimeUtility::GetTimeSinceEpoch();

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
		VT_ASSERT_MSG(!m_entityRegistry.Contains(uuid), "Entity must not exist!");

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
		newEntity.GetComponent<CommonComponent>().timeCreatedID = TimeUtility::GetTimeSinceEpoch();

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
		std::scoped_lock lock{ m_removeEntityMutex };
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
		Vector<EntityID> entityStack;
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

			if (ent.HasComponent<MeshComponent>())
			{
				const auto& renderObjectsComp = ent.GetComponent<MeshComponent>();
				for (const auto& renderObjId : renderObjectsComp.renderObjectIds)
				{
					m_renderScene->InvalidateRenderObject(renderObjId);
				}
			}
		}
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
				VT_LOG(Error, "[AssetManager] Unable to read dependency file!");
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
				VT_LOG(Error, "[AssetManager] Dependency list contains invalid YAML! Please correct it! Error: {0}", e.what());
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

		VT_LOG(Info, "[Scene] {0} assets has been queued for scene {1}!", dependencies.size(), scenePath.string());
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
				auto ent = newScene->CreateEntity("Cube");

				//auto id = ent.GetComponent<IDComponent>();

				auto& meshComp = ent.AddComponent<MeshComponent>();
				meshComp.handle = AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cube_Mesh.vtasset");

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

				ent.SetPosition({ 0.f, 0.f, -500.f });
			}
		}

		newScene->InvalidateRenderScene();
		newScene->m_sceneSettings.useWorldEngine = true;

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

			otherScene->GetWorldEngineMutable().OnEntityMoved(entity);
		});

		otherScene->InvalidateRenderScene();
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

	const Vector<Entity> Scene::FlattenEntityHeirarchy(Entity entity)
	{
		Vector<Entity> result;
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
		Vector<Entity> hierarchy{};
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
			const auto& transComp = m_registry.get<TransformComponent>(ent.GetHandle());

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

	void Scene::Initialize()
	{
		m_visionSystem = CreateRef<Vision>(this);
		m_renderScene = CreateRef<RenderScene>(this);

		ComponentRegistry::Helpers::SetupComponentCallbacks(m_registry);

		AddLayer("Main", 0);

		m_worldEngine.Reset(this, 16, 4);

		GetECSSystemRegistry().Build(m_ecsBuilder);
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

		m_entityRegistry.RemoveEntity(entity);
		m_registry.destroy(entity);
	}

	void Scene::AddLayer(const std::string& layerName, uint32_t layerId)
	{
		m_sceneLayers.emplace_back(layerId, layerName);
	}

	void Scene::SortScene()
	{
		m_registry.sort<CommonComponent>([&](const entt::entity lhs, const entt::entity rhs)
		{
			const auto& lhsCommonComp = m_registry.get<CommonComponent>(lhs);
			const auto& rhsCommonComp = m_registry.get<CommonComponent>(rhs);

			return lhsCommonComp.timeCreatedID < rhsCommonComp.timeCreatedID;
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

	const Vector<Entity> Scene::GetAllEntities() const
	{
		Vector<Entity> result{};
		result.reserve(m_registry.alive());

		m_registry.each([&](const entt::entity id)
		{
			result.emplace_back(Entity{ id, const_cast<Scene*>(this) }); // Doubtable
		});

		return result;
	}

	const Vector<Entity> Scene::GetAllEditedEntities() const
	{
		Vector<Entity> entities;

		for (const auto& entity : m_entityRegistry.GetEditedEntities())
		{
			entities.push_back(GetEntityFromUUID(entity));
		}

		return entities;
	}

	const Vector<EntityID> Scene::GetAllRemovedEntities() const
	{
		Vector<EntityID> entities;

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
			if (!mesh)
			{
				continue;
			}

			const auto& materialTable = mesh->GetMaterialTable();

			for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
			{
				const auto materialIndex = mesh->GetSubMeshes().at(i).materialIndex;

				Ref<Material> mat = AssetManager::QueueAsset<Material>(materialTable.GetMaterial(materialIndex));
				if (!mat)
				{
					VT_LOG(Warning, "[MeshComponent]: Mesh {} has an invalid material at index {}!", mesh->assetName, materialIndex);
					mat = Renderer::GetDefaultResources().defaultMaterial;
				}

				if (static_cast<uint32_t>(meshComp.materials.size()) > materialIndex)
				{
					if (meshComp.materials.at(materialIndex) != mat->handle)
					{
						Ref<Material> tempMat = AssetManager::QueueAsset<Material>(meshComp.materials.at(materialIndex));
						mat = tempMat;
					}
				}

				auto uuid = m_renderScene->Register(idComp.id, mesh, mat, static_cast<uint32_t>(i));
				meshComp.renderObjectIds.emplace_back(uuid);
			}
		}
	}
}
