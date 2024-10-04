#include "vtpch.h"
#include "Volt/Scene/Scene.h"

#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Scene/Entity.h"

#include "Volt/Components/AudioComponents.h"
#include "Volt/Components/LightComponents.h"
#include "Volt/Components/RenderingComponents.h"

#include "Volt/Animation/AnimationManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

#include "Volt/Math/Math.h"

#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Vision/Vision.h"

#include <AssetSystem/AssetManager.h>

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

	void Scene::OnRuntimeStart()
	{
		Physics::CreateScene(this);
		Physics::CreateActors(this);
		AnimationManager::Reset();

		m_isPlaying = true;
		m_timeSinceStart = 0.f;

		m_entityScene.OnRuntimeStart();
	}

	void Scene::OnRuntimeEnd()
	{
		m_entityScene.OnRuntimeEnd();
		m_isPlaying = false;

		Physics::DestroyScene();
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
		m_statistics.entityCount = m_entityScene.GetEntityAliveCount();
		
		m_entityScene.Update(aDeltaTime);

		AnimationManager::Update(aDeltaTime);
		Physics::GetScene()->Simulate(aDeltaTime);
		m_visionSystem->Update(aDeltaTime);

		m_timeSinceStart += aDeltaTime;
		m_currentDeltaTime = aDeltaTime;
	}

	void Scene::FixedUpdate(float aDeltaTime)
	{
		m_entityScene.FixedUpdate(aDeltaTime);
	}

	void Scene::UpdateEditor(float aDeltaTime)
	{
		VT_PROFILE_FUNCTION();

		m_statistics.entityCount = m_entityScene.GetEntityAliveCount();

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

		m_statistics.entityCount = m_entityScene.GetEntityAliveCount();
	}

	void Scene::SortScene()
	{
		m_entityScene.SortScene();
	}

	Entity Scene::CreateEntity(const std::string& tag)
	{
		EntityHelper newHelper = m_entityScene.CreateEntity(tag);
		VT_ENSURE(newHelper);

		Entity newEntity(newHelper.GetHandle(), this);
		m_worldEngine.AddEntity(newEntity);

		return newEntity;
	}

	Entity Scene::CreateEntityWithID(const EntityID& id, const std::string& tag)
	{
		EntityHelper newHelper = m_entityScene.CreateEntityWithID(id, tag);
		VT_ENSURE(newHelper);

		Entity newEntity(newHelper.GetHandle(), this);
		m_worldEngine.AddEntity(newEntity);

		return newEntity;
	}

	Entity Scene::GetEntityFromID(const EntityID id) const
	{
		EntityHelper helper = m_entityScene.GetEntityHelperFromEntityID(id);
		return Entity(helper.GetHandle(), const_cast<Scene*>(this));
	}

	Entity Scene::GetEntityFromHandle(entt::entity entityHandle) const
	{
		EntityHelper helper = m_entityScene.GetEntityHelperFromEntityHandle(entityHandle);
		return Entity(helper.GetHandle(), const_cast<Scene*>(this));
	}

	EntityHelper Scene::GetEntityHelperFromEntityID(EntityID entityId) const
	{
		return m_entityScene.GetEntityHelperFromEntityID(entityId);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_entityScene.DestroyEntity(entity.GetID());
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

	void Scene::InvalidateEntityTransform(const EntityID& entityId)
	{
		Vector<EntityID> invalidatedEntities = m_entityScene.InvalidateEntityTransform(entityId);

		for (const auto& id : invalidatedEntities)
		{
			Entity currentEntity = GetEntityFromID(id);

			if (m_sceneSettings.useWorldEngine)
			{
				m_worldEngine.OnEntityMoved(currentEntity);
			}
		}
	}

	bool Scene::IsEntityValid(EntityID entityId) const
	{
		return m_entityScene.IsEntityValid(entityId);
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
				meshComp.handle = AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Cube.vtasset");
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

		auto& registry = m_entityScene.GetRegistry();

		registry.each([&](entt::entity id)
		{
			const EntityID uuid = registry.get<IDComponent>(id).id;

			auto entity =  otherScene->CreateEntityWithID(uuid);
			Entity::Copy(Entity{ id, this }, entity, EntityCopyFlags::None);

			otherScene->InvalidateEntityTransform(entity.GetID());
			otherScene->GetWorldEngineMutable().OnEntityMoved(entity);
		});

		otherScene->InvalidateRenderScene();
	} 

	void Scene::Clear()
	{
		m_entityScene.ClearScene();
	}

	glm::mat4 Scene::GetWorldTransform(Entity entity) const
	{
		const TQS entityWorldTQS = m_entityScene.GetEntityWorldTQS(m_entityScene.GetEntityHelperFromEntityID(entity.GetID()));

		const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, entityWorldTQS.translation)
			* glm::mat4_cast(entityWorldTQS.rotation)
			* glm::scale(glm::mat4{ 1.f }, entityWorldTQS.scale);

		return transform;
	}

	Vector<Entity> Scene::FlattenEntityHeirarchy(Entity entity)
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

	void Scene::Initialize()
	{
		m_visionSystem = CreateRef<Vision>(this);
		m_renderScene = CreateRef<RenderScene>(this);

		m_worldEngine.Reset(this, 16, 4);
	}

	bool Scene::IsRelatedTo(Entity entity, Entity otherEntity)
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
				Entity child = GetEntityFromID(childId);
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

	void Scene::MarkEntityAsEdited(const Entity& entity)
	{
		m_entityScene.MarkEntityAsEdited(m_entityScene.GetEntityHelperFromEntityID(entity.GetID()));
	}

	void Scene::ClearEditedEntities()
	{
		m_entityScene.ClearEditedEntities();
	}

	Vector<Entity> Scene::GetAllEntities() const
	{
		const auto& registry = m_entityScene.GetRegistry();

		Vector<Entity> result{};
		result.reserve(registry.alive());

		registry.each([&](const entt::entity id)
		{
			result.emplace_back(Entity{ id, const_cast<Scene*>(this) });
		});

		return result;
	}

	Vector<Entity> Scene::GetAllEditedEntities() const
	{
		Vector<Entity> entities;

		for (const auto& entity : m_entityScene.GetEditedEntities())
		{
			entities.push_back(GetEntityFromID(entity));
		}

		return entities;
	}

	Vector<EntityID> Scene::GetAllRemovedEntities() const
	{
		Vector<EntityID> entities;

		for (const auto& entity : m_entityScene.GetRemovedEntities())
		{
			entities.push_back(entity);
		}

		return entities;
	}
	
	void Scene::InvalidateRenderScene()
	{
		auto& registry = m_entityScene.GetRegistry();

		const auto& meshView = registry.view<MeshComponent, IDComponent>();
		for (const auto& id : meshView)
		{
			auto& meshComp = registry.get<MeshComponent>(id);
			auto& idComp = registry.get<IDComponent>(id);

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

	TQS Scene::GetEntityWorldTQS(const Entity& entity) const
	{
		return m_entityScene.GetEntityWorldTQS(m_entityScene.GetEntityHelperFromEntityID(entity.GetID()));
	}
}
