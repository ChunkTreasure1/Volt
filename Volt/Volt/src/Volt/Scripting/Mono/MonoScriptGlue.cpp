#include "vtpch.h"
#include "MonoScriptGlue.h"
#include "Volt/Net/Serialization/NetSerialization.h"

#include "Volt/Events/SettingsEvent.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Scripting/Mono/MonoScriptEntity.h"
#include "Volt/Scripting/Mono/MonoGCManager.h"

#include "Volt/Rendering/DebugRenderer.h"
#include "Volt/Rendering/UIRenderer.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/Text/Font.h"
#include "Volt/Asset/Rendering/PostProcessingStack.h"
#include "Volt/Asset/Rendering/PostProcessingMaterial.h"

#include "Volt/Utility/Noise.h"

#include "Volt/Input/Input.h"
#include "Volt/Components/Components.h"
#include "Volt/Components/NavigationComponents.h"
#include "Volt/Components/PhysicsComponents.h"
#include "Volt/Components/AudioComponents.h"
#include "Volt/Components/LightComponents.h"

#include "Volt/Vision/Vision.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Physics/PhysicsActor.h"
#include "Volt/Physics/PhysicsControllerActor.h"
#include "Volt/Physics/PhysicsShapes.h"

#include "Volt/Asset/Prefab.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Video/Video.h"
#include "Volt/Animation/AnimationController.h"

#include "Volt/Steam/SteamImplementation.h"

#include <Navigation/Core/NavigationSystem.h>

#include <GraphKey/Graph.h>

#include "Volt/Project/ProjectManager.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "Volt/Net/Client/NetClient.h"
#include "Volt/Net/SceneInteraction/NetActorComponent.h"
#include <Nexus/Interface/NetManager/NetManager.h>
#include <Nexus/Winsock/AddressHelpers.hpp>

namespace Volt
{
#define VT_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volt.InternalCalls::" #Name, Name)

#pragma region VoltApplication

	inline static bool VoltApplication_IsRuntime()
	{
		return Volt::Application::Get().IsRuntime();
	}

	inline static void VoltApplication_SetResolution(uint32_t aResolutionX, uint32_t aResolutionY)
	{
		Volt::WindowResizeEvent event{ aResolutionX, aResolutionY };
		Volt::Application::Get().OnEvent(event);
	}

	inline static void VoltApplication_SetWindowMode(uint32_t aWindowMode)
	{
		Volt::Application::Get().GetWindow().SetWindowMode((Volt::WindowMode)aWindowMode);
	}

	inline static void VoltApplication_LoadLevel(MonoString* aLevelAssetPath)
	{
		std::string levelPath = MonoScriptUtils::GetStringFromMonoString(aLevelAssetPath);
		Volt::AssetHandle aHandle = Volt::AssetManager::Get().GetAssetHandleFromFilePath(levelPath);
		Volt::OnSceneTransitionEvent loadEvent{ aHandle };
		Volt::Application::Get().OnEvent(loadEvent);
	}

	inline static void VoltApplication_Quit()
	{
		Volt::WindowCloseEvent loadEvent{};
		Volt::Application::Get().OnEvent(loadEvent);
	}

	inline static MonoString* VoltApplication_GetClipboard()
	{
		return MonoScriptUtils::GetMonoStringFromString(Volt::Application::Get().GetWindow().GetClipboard());
	}

	inline static void VoltApplication_SetClipboard(MonoString* string)
	{
		Volt::Application::Get().GetWindow().SetClipboard(MonoScriptUtils::GetStringFromMonoString(string));
	}

#pragma endregion


#pragma region Entity
	inline static bool Entity_IsValid(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		return scene->GetRegistry().Exists(entityId);
	}

	inline static bool Entity_HasComponent(entt::entity entityId, MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto compName = MonoScriptUtils::GetStringFromMonoString(componentType);
		return scene->GetRegistry().HasComponent(Wire::ComponentRegistry::GetRegistryDataFromName(compName).guid, entityId);
	}

	inline static bool Entity_HasScript(entt::entity entityId, MonoString* scriptType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto scriptName = MonoScriptUtils::GetStringFromMonoString(scriptType);
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return false;
		}

		if (!entity.HasComponent<MonoScriptComponent>())
		{
			return false;
		}

		auto wantedMonoScriptClass = MonoScriptEngine::GetScriptClass(scriptName);
		if (!wantedMonoScriptClass)
		{
			return false;
		}

		for (const auto& script : entity.GetComponent<MonoScriptComponent>().scriptNames)
		{
			auto currentMonoScriptClass = MonoScriptEngine::GetScriptClass(script);
			if (!currentMonoScriptClass)
			{
				continue;
			}

			if (script == scriptName || currentMonoScriptClass->IsSubclassOf(wantedMonoScriptClass))
			{
				return true;
			}
		}

		return false;
	}

	inline static MonoObject* Entity_GetScript(entt::entity entityId, MonoString* scriptType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto scriptName = MonoScriptUtils::GetStringFromMonoString(scriptType);
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return nullptr;
		}

		if (!entity.HasComponent<MonoScriptComponent>())
		{
			return nullptr;
		}

		auto wantedMonoScriptClass = MonoScriptEngine::GetScriptClass(scriptName);
		if (!wantedMonoScriptClass)
		{
			return nullptr;
		}

		auto& comp = entity.GetComponent<MonoScriptComponent>();

		for (uint32_t i = 0; const auto & script : comp.scriptNames)
		{
			auto currentMonoScriptClass = MonoScriptEngine::GetScriptClass(script);
			if (!currentMonoScriptClass)
			{
				continue;
			}

			if (script == scriptName || currentMonoScriptClass->IsSubclassOf(wantedMonoScriptClass))
			{
				auto instance = MonoScriptEngine::GetInstanceFromId(comp.scriptIds.at(i));

				if (instance)
				{
					auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
					return monoObject;
				}
			}

			i++;
		}

		return nullptr;
	}

	inline static void Entity_RemoveComponent(entt::entity entityId, MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto compName = MonoScriptUtils::GetStringFromMonoString(componentType);
		scene->GetRegistry().RemoveComponent(Wire::ComponentRegistry::GetRegistryDataFromName(compName).guid, entityId);
	}

	inline static void Entity_RemoveScript(entt::entity entityId, UUID scriptId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		auto& scriptsIdList = scene->GetRegistry().GetComponent<MonoScriptComponent>(entityId).scriptIds;
		auto& scriptsNamesList = scene->GetRegistry().GetComponent<MonoScriptComponent>(entityId).scriptNames;

		auto it = std::find(scriptsIdList.begin(), scriptsIdList.end(), scriptId);
		int32_t index = (int32_t)std::distance(scriptsIdList.begin(), it);

		scriptsIdList.erase(scriptsIdList.begin() + index);
		scriptsNamesList.erase(scriptsNamesList.begin() + index);

		MonoScriptEngine::OnDestroyInstance(scriptId);
	}

	inline static void Entity_AddComponent(entt::entity entityId, MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto compName = MonoScriptUtils::GetStringFromMonoString(componentType);
		scene->GetRegistry().AddComponent(Wire::ComponentRegistry::GetRegistryDataFromName(compName).guid, entityId);
	}

	inline static void Entity_AddScript(entt::entity entityId, MonoString* scriptType, UUID* outScriptId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto scriptName = MonoScriptUtils::GetStringFromMonoString(scriptType);
		if (!scene->GetRegistry().HasComponent<MonoScriptComponent>(entityId))
		{
			scene->GetRegistry().AddComponent<MonoScriptComponent>(entityId);
		}

		auto sid = scene->GetRegistry().GetComponent<MonoScriptComponent>(entityId).scriptIds.emplace_back(UUID());
		auto sname = scene->GetRegistry().GetComponent<MonoScriptComponent>(entityId).scriptNames.emplace_back(scriptName);

		MonoScriptEngine::OnAwakeInstance(sid, entityId, sname);
		MonoScriptEngine::QueueOnCreate(sid);

		*outScriptId = sid;
	}

	inline static MonoObject* Entity_FindByName(MonoString* name)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto entityName = MonoScriptUtils::GetStringFromMonoString(name);
		Entity ent = scene->GetEntityWithName(entityName);

		if (scene->GetRegistry().Exists(ent.GetID()))
		{
			auto instance = MonoScriptEngine::GetEntityFromId(ent.GetID());
			if (!instance)
			{
				instance = MonoScriptEngine::GetOrCreateMonoEntity(ent.GetID());
			}

			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			return monoObject;
		}
		return nullptr;
	}

	inline static MonoObject* Entity_FindById(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		if (scene->GetRegistry().Exists(entityId))
		{
			auto instance = MonoScriptEngine::GetEntityFromId(entityId);
			if (!instance)
			{
				instance = MonoScriptEngine::GetOrCreateMonoEntity(entityId);
			}

			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			return monoObject;
		}
		return nullptr;
	}

	inline static MonoObject* Entity_CreateNewEntity(MonoString* name)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto entityName = MonoScriptUtils::GetStringFromMonoString(name);
		Entity ent = scene->CreateEntity(entityName);
		MonoScriptEngine::GetOrCreateMonoEntity(ent.GetID());

		auto instance = MonoScriptEngine::GetEntityFromId(ent.GetID());

		if (instance)
		{
			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			return monoObject;
		}

		return nullptr;
	}

	inline static void RecursiveCreatePrefabEntities(entt::entity entId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		Volt::MonoScriptEngine::GetOrCreateMonoEntity(entId);
		if (scene->GetRegistry().HasComponent<Volt::MonoScriptComponent>(entId))
		{
			auto& comp = scene->GetRegistry().GetComponent<Volt::MonoScriptComponent>(entId);
			for (uint32_t i = 0; const auto & sid : comp.scriptIds)
			{
				Volt::MonoScriptEngine::OnAwakeInstance(sid, entId, comp.scriptNames[i]);
				Volt::MonoScriptEngine::QueueOnCreate(sid);
				i++;
			}

		}
		auto children = scene->GetRegistry().GetComponent<Volt::RelationshipComponent>(entId).Children;

		for (auto child : children)
		{
			RecursiveCreatePrefabEntities(child);
		}
	}

	inline static MonoObject* Entity_CreateNewEntityWithPrefab(uint64_t handle)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		auto prefab = Volt::AssetManager::GetAsset<Volt::Prefab>(handle);
		if (!prefab || !prefab->IsValid() || Volt::AssetManager::GetAssetTypeFromHandle(handle) != AssetType::Prefab)
		{
			return nullptr;
		}

		auto entityId = prefab->Instantiate(scene);
		RecursiveCreatePrefabEntities(entityId);

		auto instance = MonoScriptEngine::GetEntityFromId(entityId);

		if (instance)
		{
			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			return monoObject;
		}

		return nullptr;
	}

	inline static void Entity_DeleteEntity(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		Volt::Entity entity{ entityId, scene };

		if (entity)
		{
			scene->RemoveEntity(entity);
		}
		else
		{
			VT_WARN("Trying to remove invalid entity with id {0}!", entityId);
		}
	}

	inline static MonoObject* Entity_Clone(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		auto& registry = scene->GetRegistry();

		Entity ent = scene->CreateEntity();

		Volt::Entity::Copy(registry, registry, scene->GetScriptFieldCache(), scene->GetScriptFieldCache(), entityId, ent.GetID(), { Volt::RigidbodyComponent::comp_guid, Volt::CharacterControllerComponent::comp_guid }, true);

		if (registry.HasComponent<Volt::RigidbodyComponent>(entityId))
		{
			const auto srcComp = registry.GetComponent<Volt::RigidbodyComponent>(entityId);
			registry.AddComponent<Volt::RigidbodyComponent>(ent.GetID(), srcComp.bodyType, srcComp.layerId, srcComp.mass, srcComp.linearDrag, srcComp.lockFlags, srcComp.angularDrag, srcComp.disableGravity, srcComp.isKinematic, srcComp.collisionType);
		}

		if (registry.HasComponent<Volt::CharacterControllerComponent>(entityId))
		{
			const auto srcComp = registry.GetComponent<Volt::CharacterControllerComponent>(entityId);
			registry.AddComponent<Volt::CharacterControllerComponent>(ent.GetID(), srcComp.climbingMode, srcComp.slopeLimit, srcComp.invisibleWallHeight, srcComp.maxJumpHeight, srcComp.contactOffset, srcComp.stepOffset, srcComp.density, srcComp.layer, srcComp.hasGravity);
		}

		ent.ResetChildren();

		MonoScriptEngine::GetOrCreateMonoEntity(ent.GetID());

		if (scene->GetRegistry().HasComponent<Volt::MonoScriptComponent>(ent.GetID()))
		{
			auto& comp = scene->GetRegistry().GetComponent<Volt::MonoScriptComponent>(ent.GetID());
			for (uint32_t i = 0; const auto & sid : comp.scriptIds)
			{
				MonoScriptEngine::OnCreateInstance(sid, ent.GetID(), comp.scriptNames[i]);
				MonoScriptEngine::QueueOnCreate(sid);
				i++;
			}
		}

		auto instance = MonoScriptEngine::GetEntityFromId(ent.GetID());

		if (instance)
		{
			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			return monoObject;
		}

		return nullptr;
	}
#pragma endregion

#pragma region Asset

	inline static bool AssetManager_HasAsset(uint64_t handle)
	{
		if (Volt::AssetManager::ExistsInRegistry(handle))
		{
			return true;
		}
		return false;
	}

	inline static uint64_t AssetManager_GetAssetHandleFromPath(MonoString* string)
	{
		const std::string str = MonoScriptUtils::GetStringFromMonoString(string);
		return Volt::AssetManager::GetAssetHandleFromFilePath(str);
	}

#pragma endregion

#pragma region Scene
	inline static void Scene_Load(uint64_t handle)
	{
		if (Volt::AssetManager::GetAssetTypeFromHandle(handle) != AssetType::Scene)
		{
			VT_CORE_ERROR("Unable to load scene with handle {0}, because it's not a scene!", handle);
			return;
		}

		OnSceneTransitionEvent scene(handle);
		Application::Get().OnEvent(scene);
	}

	inline static void Scene_Preload(MonoString* path)
	{
		std::string str = MonoScriptUtils::GetStringFromMonoString(path);

		Scene::PreloadSceneAssets(str);
	}

	inline static void Scene_Save(uint64_t handle)
	{
		if (Volt::AssetManager::GetAssetTypeFromHandle(handle) != AssetType::Scene)
		{
			VT_CORE_ERROR("Unable to load scene with handle {0}, because it's not a scene!", handle);
			return;
		}

		Volt::AssetManager::SaveAsset(Volt::AssetManager::Get().GetAssetRaw(handle));
	}

	inline static MonoArray* Scene_GetAllEntities()
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		auto entities = scene->GetAllEntitiesWith<TransformComponent>();
		return MonoScriptUtils::CreateMonoArrayEntity(entities);
	}

	inline static MonoArray* Scene_GetAllEntitiesWithComponent(MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		const auto compName = MonoScriptUtils::GetStringFromMonoString(componentType);

		auto compGuid = Wire::ComponentRegistry::GetRegistryDataFromName(compName).guid;
		auto entities = scene->GetRegistry().GetComponentView(compGuid);

		return MonoScriptUtils::CreateMonoArrayEntity(entities);
	}

	inline static MonoArray* Scene_GetAllEntitiesWithScript(MonoString* scriptType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		auto& registry = scene->GetRegistry();

		const auto scriptName = MonoScriptUtils::GetStringFromMonoString(scriptType);
		auto entities = scene->GetAllEntitiesWith<MonoScriptComponent>();

		std::vector<entt::entity> removeEntities;

		auto wantedMonoScriptClass = MonoScriptEngine::GetScriptClass(scriptName);
		if (!wantedMonoScriptClass)
		{
			return nullptr;
		}

		for (uint32_t i = 0; i < entities.size(); ++i)
		{
			if (registry.HasComponent<MonoScriptComponent>(entities[i]))
			{
				bool hasScript = false;

				for (auto& name : registry.GetComponent<MonoScriptComponent>(entities[i]).scriptNames)
				{
					auto currentMonoScriptClass = MonoScriptEngine::GetScriptClass(name);
					if (!currentMonoScriptClass)
					{
						continue;
					}

					if (name == scriptName || currentMonoScriptClass->IsSubclassOf(wantedMonoScriptClass))
					{
						hasScript = true;
						break;
					}
				}

				if (!hasScript)
				{
					removeEntities.emplace_back(i);
				}
			}
		}

		std::sort(removeEntities.rbegin(), removeEntities.rend());

		for (auto i : removeEntities)
		{
			entities.erase(entities.begin() + i);
		}

		return MonoScriptUtils::CreateMonoArrayEntity(entities);
	}

	inline static MonoObject* Scene_InstantiateSplitMesh(uint64_t meshHandle)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		auto entity = scene->InstantiateSplitMesh(meshHandle);
		auto monoEntity = MonoScriptEngine::GetOrCreateMonoEntity(entity.GetID());

		return MonoGCManager::GetObjectFromHandle(monoEntity->GetHandle());
	}

	inline static void Scene_CreateDynamicPhysicsActor(entt::entity id)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ id, scene };

		entity.AddComponent<RigidbodyComponent>(BodyType::Dynamic);
	}
#pragma endregion

#pragma region TransformComponent
	inline static void TransformComponent_GetPosition(entt::entity entityId, glm::vec3* outPosition)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outPosition = entity.GetPosition();
	}

	inline static void TransformComponent_SetPosition(entt::entity entityId, glm::vec3* translation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		entity.SetPosition(*translation);
	}

	inline static void TransformComponent_GetRotation(entt::entity entityId, glm::quat* outRotation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outRotation = entity.GetRotation();
	}

	inline static void TransformComponent_SetRotation(entt::entity entityId, glm::quat* rotation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		entity.SetRotation(*rotation);
	}

	inline static void TransformComponent_GetScale(entt::entity entityId, glm::vec3* outScale)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outScale = entity.GetLocalScale();
	}

	inline static void TransformComponent_SetScale(entt::entity entityId, glm::vec3* scale)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		entity.SetLocalScale(*scale);
	}

	inline static void TransformComponent_GetForward(entt::entity entityId, glm::vec3* outForward)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outForward = entity.GetForward();
	}

	inline static void TransformComponent_GetRight(entt::entity entityId, glm::vec3* outRight)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outRight = entity.GetRight();
	}

	inline static void TransformComponent_GetUp(entt::entity entityId, glm::vec3* outUp)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outUp = entity.GetUp();
	}

	inline static void TransformComponent_GetLocalPosition(entt::entity entityId, glm::vec3* outPosition)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outPosition = entity.GetLocalPosition();
	}

	inline static void TransformComponent_SetLocalPosition(entt::entity entityId, glm::vec3* translation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		entity.SetLocalPosition(*translation);
	}

	inline static void TransformComponent_GetLocalRotation(entt::entity entityId, glm::quat* outRotation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outRotation = entity.GetLocalRotation();
	}

	inline static void TransformComponent_SetLocalRotation(entt::entity entityId, glm::quat* rotation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		entity.SetLocalRotation(*rotation);
	}

	inline static void TransformComponent_GetLocalScale(entt::entity entityId, glm::vec3* outScale)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outScale = entity.GetLocalScale();
	}

	inline static void TransformComponent_SetLocalScale(entt::entity entityId, glm::vec3* scale)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		entity.SetLocalScale(*scale);
	}

	inline static void TransformComponent_GetLocalForward(entt::entity entityId, glm::vec3* outForward)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outForward = entity.GetLocalForward();
	}

	inline static void TransformComponent_GetLocalRight(entt::entity entityId, glm::vec3* outRight)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outRight = entity.GetLocalRight();
	}

	inline static void TransformComponent_GetLocalUp(entt::entity entityId, glm::vec3* outUp)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		*outUp = entity.GetLocalUp();
	}

	inline static bool TransformComponent_GetVisible(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return false;
		}

		return entity.IsVisible();
	}

	inline static void TransformComponent_SetVisible(entt::entity entityId, bool value)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		entity.SetVisible(value);
	}
#pragma endregion

#pragma region TagComponent
	inline static void TagComponent_SetTag(entt::entity entityId, MonoString* tag)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TransformComponent>())
		{
			return;
		}

		const auto str = MonoScriptUtils::GetStringFromMonoString(tag);
		entity.GetComponent<TagComponent>().tag = str;
	}

	inline static MonoString* TagComponent_GetTag(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity)
		{
			return nullptr;
		}

		if (!entity.HasComponent<TagComponent>())
		{
			return nullptr;
		}

		return MonoScriptUtils::GetMonoStringFromString(entity.GetComponent<TagComponent>().tag);
	}
#pragma endregion

#pragma region Log
	inline static void Log_String(MonoString* string, LogLevel logLevel)
	{
		if (string == nullptr) { return; }
		const auto str = MonoScriptUtils::GetStringFromMonoString(string);

		switch (logLevel)
		{
			case LogLevel::Trace:
				VT_CORE_TRACE(str);
				break;
			case LogLevel::Info:
				VT_CORE_INFO(str);
				break;
			case LogLevel::Warning:
				VT_CORE_WARN(str);
				break;
			case LogLevel::Error:
				VT_CORE_ERROR(str);
				break;
			case LogLevel::Critical:
				VT_CORE_CRITICAL(str);
				break;
		}
	}
#pragma endregion

#pragma region RelationshipComponent
	inline static MonoArray* RelationshipComponent_GetChildren(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		return MonoScriptUtils::CreateMonoArrayEntity(scene->GetRegistry().GetComponent<RelationshipComponent>(entityId).Children);
	}

	inline static MonoObject* RelationshipComponent_FindByName(entt::entity entityId, MonoString* name)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		auto& registry = scene->GetRegistry();
		const auto entityName = MonoScriptUtils::GetStringFromMonoString(name);

		if (!registry.HasComponent<RelationshipComponent>(entityId)) { return nullptr; }

		entt::entity childId = 0;
		for (const auto& c : registry.GetComponent<RelationshipComponent>(entityId).Children)
		{
			if (registry.HasComponent<TagComponent>(c))
			{
				if (registry.GetComponent<TagComponent>(c).tag == entityName)
				{
					childId = c;
				}
			}
		}

		if (registry.Exists(childId))
		{
			auto instance = MonoScriptEngine::GetEntityFromId(childId);
			if (!instance)
			{
				instance = MonoScriptEngine::GetOrCreateMonoEntity(childId);
			}

			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			return monoObject;
		}
		return nullptr;
	}

	inline static MonoObject* RelationshipComponent_GetParent(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.IsNull())
		{
			return nullptr;
		}

		auto monoEntity = MonoScriptEngine::GetEntityFromId(entity.GetParent().GetID());

		if (!monoEntity)
		{
			monoEntity = MonoScriptEngine::GetOrCreateMonoEntity(entity.GetParent().GetID());
		}

		auto monoObject = MonoGCManager::GetObjectFromHandle(monoEntity->GetHandle());
		return monoObject;
	}

	inline static void RelationshipComponent_SetParent(entt::entity entityId, entt::entity parentEntityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.GetParent())
		{
			scene->UnparentEntity(entity);
		}

		if (scene->GetRegistry().Exists(parentEntityId))
		{
			scene->ParentEntity(Volt::Entity{ parentEntityId, scene }, entity);
		}
	}
#pragma endregion

#pragma region RigidbodyComponent
	inline static BodyType RigidbodyComponent_GetBodyType(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().bodyType;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return BodyType::Static;
	}

	inline static void RigidbodyComponent_SetBodyType(entt::entity entityId, BodyType* bodyType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		auto& comp = entity.GetComponent<RigidbodyComponent>();
		if (*bodyType != comp.bodyType)
		{
			auto actor = Physics::GetScene()->GetActor(entity);
			if (actor)
			{
				Physics::GetScene()->RemoveActor(actor);
			}

			Physics::GetScene()->CreateActor(entity);
			comp.bodyType = *bodyType;
		}
	}

	inline static uint32_t RigidbodyComponent_GetLayerId(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().layerId;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return 0;
	}

	inline static void RigidbodyComponent_SetLayerId(entt::entity entityId, uint32_t* layerId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		entity.GetComponent<RigidbodyComponent>().layerId = *layerId;

		auto actor = Physics::GetScene()->GetActor(entity);
		if (!actor)
		{
			return;
		}

		actor->SetSimulationData(*layerId);
	}

	inline static float RigidbodyComponent_GetMass(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().mass;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return 0.f;
	}

	inline static void RigidbodyComponent_SetMass(entt::entity entityId, float* mass)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		entity.GetComponent<RigidbodyComponent>().mass = *mass;

		auto actor = Physics::GetScene()->GetActor(entity);
		if (!actor)
		{
			return;
		}

		actor->SetMass(*mass);
	}

	inline static void RigidbodyComponent_SetLinearDrag(entt::entity entityId, float* linearDrag)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		entity.GetComponent<RigidbodyComponent>().linearDrag = *linearDrag;

		auto actor = Physics::GetScene()->GetActor(entity);
		if (!actor)
		{
			return;
		}

		actor->SetLinearDrag(*linearDrag);
	}

	inline static float RigidbodyComponent_GetLinearDrag(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().linearDrag;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return 0.f;
	}

	inline static void RigidbodyComponent_SetAngularDrag(entt::entity entityId, float* angularDrag)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		entity.GetComponent<RigidbodyComponent>().angularDrag = *angularDrag;

		auto actor = Physics::GetScene()->GetActor(entity);
		if (!actor)
		{
			return;
		}

		actor->SetAngularDrag(*angularDrag);
	}

	inline static float RigidbodyComponent_GetAngularDrag(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().angularDrag;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return 0.f;
	}

	inline static uint32_t RigidbodyComponent_GetLockFlags(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().lockFlags;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return 0;
	}

	inline static void RigidbodyComponent_SetLockFlags(entt::entity entityId, uint32_t* flags)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		entity.GetComponent<RigidbodyComponent>().lockFlags = *flags;

		auto actor = Physics::GetScene()->GetActor(entity);
		if (!actor)
		{
			return;
		}

		actor->SetLockFlags(*flags);
	}

	inline static bool RigidbodyComponent_GetDisableGravity(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().disableGravity;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return false;
	}

	inline static void RigidbodyComponent_SetDisableGravity(entt::entity entityId, bool* state)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		entity.GetComponent<RigidbodyComponent>().disableGravity = *state;

		auto actor = Physics::GetScene()->GetActor(entity);
		if (!actor)
		{
			return;
		}

		actor->SetGravityDisabled(*state);
	}

	inline static bool RigidbodyComponent_GetIsKinematic(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().isKinematic;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return false;
	}

	inline static void RigidbodyComponent_SetIsKinematic(entt::entity entityId, bool* state)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<RigidbodyComponent>())
		{
			return;
		}

		entity.GetComponent<RigidbodyComponent>().isKinematic = *state;

		auto actor = Physics::GetScene()->GetActor(entity);
		if (!actor)
		{
			return;
		}

		actor->SetKinematic(*state);
	}

	inline static CollisionDetectionType RigidbodyComponent_GetCollisionDetectionType(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			return entity.GetComponent<RigidbodyComponent>().collisionType;
		}

		VT_CORE_ERROR("Entity {0} does not have a RigidbodyComponent!", entityId);
		return CollisionDetectionType::Discrete;
	}

	inline static void RigidbodyComponent_SetCollisionDetectionType(entt::entity entityId, CollisionDetectionType* collisionDetectionType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().collisionType = *collisionDetectionType;
		}
	}
#pragma endregion

#pragma region CharacterControllerComponent
	inline static float CharacterControllerComponent_GetSlopeLimit(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<CharacterControllerComponent>())
		{
			return entity.GetComponent<CharacterControllerComponent>().slopeLimit;
		}

		VT_CORE_ERROR("Entity {0} does not have a CharacterControllerComponent!", entityId);
		return 0.f;
	}

	inline static void CharacterControllerComponent_SetSlopeLimit(entt::entity entityId, float slopeLimit)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (!entity.HasComponent<CharacterControllerComponent>())
		{
			return;
		}

		entity.GetComponent<CharacterControllerComponent>().slopeLimit = slopeLimit;
	}

	inline static float CharacterControllerComponent_GetInvisibleWallHeight(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			return entity.GetComponent<CharacterControllerComponent>().invisibleWallHeight;
		}

		VT_CORE_ERROR("Entity {0} does not have a CharacterControllerComponent!", entityId);
		return 0.f;
	}

	inline static void CharacterControllerComponent_SetInvisibleWallHeight(entt::entity entityId, float invisibleWallHeight)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			entity.GetComponent<CharacterControllerComponent>().invisibleWallHeight = invisibleWallHeight;
		}
	}

	inline static float CharacterControllerComponent_GetMaxJumpHeight(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			return entity.GetComponent<CharacterControllerComponent>().maxJumpHeight;
		}

		VT_CORE_ERROR("Entity {0} does not have a CharacterControllerComponent!", entityId);
		return 0.f;
	}

	inline static void CharacterControllerComponent_SetMaxJumpHeight(entt::entity entityId, float maxJumpHeight)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			entity.GetComponent<CharacterControllerComponent>().maxJumpHeight = maxJumpHeight;
		}
	}

	inline static float CharacterControllerComponent_GetContactOffset(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			return entity.GetComponent<CharacterControllerComponent>().contactOffset;
		}

		VT_CORE_ERROR("Entity {0} does not have a CharacterControllerComponent!", entityId);
		return 0.f;
	}

	inline static void CharacterControllerComponent_SetContactOffset(entt::entity entityId, float contactOffset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			entity.GetComponent<CharacterControllerComponent>().contactOffset = contactOffset;
		}
	}

	inline static float CharacterControllerComponent_GetStepOffset(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			return entity.GetComponent<CharacterControllerComponent>().stepOffset;
		}

		VT_CORE_ERROR("Entity {0} does not have a CharacterControllerComponent!", entityId);
		return 0.f;
	}

	inline static void CharacterControllerComponent_SetStepOffset(entt::entity entityId, float stepOffset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			entity.GetComponent<CharacterControllerComponent>().stepOffset = stepOffset;
		}
	}

	inline static float CharacterControllerComponent_GetDensity(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			return entity.GetComponent<CharacterControllerComponent>().density;
		}

		VT_CORE_ERROR("Entity {0} does not have a CharacterControllerComponent!", entityId);
		return 0.f;
	}

	inline static void CharacterControllerComponent_SetDensity(entt::entity entityId, float density)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			entity.GetComponent<CharacterControllerComponent>().density = density;
		}
	}

	inline static float CharacterControllerComponent_GetGravity(entt::entity entityId)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return 0.f;
		}

		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller actor found for entity {0}!", entityId);
			return 0.f;
		}

		return actor->GetGravity();
	}

	inline static void CharacterControllerComponent_SetGravity(entt::entity entityId, float gravity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller actor found for entity {0}!", entityId);
			return;
		}

		actor->SetGravity(gravity);
	}

	inline static void CharacterControllerComponent_GetAngularVelocity(entt::entity entityId, glm::vec3* outLinearVelocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller actor found for entity {0}!", entityId);
			return;
		}

		*outLinearVelocity = actor->GetAngularVelocity();
	}

	inline static void CharacterControllerComponent_GetLinearVelocity(entt::entity entityId, glm::vec3* outLinearVelocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller actor found for entity {0}!", entityId);
			return;
		}

		*outLinearVelocity = actor->GetLinearVelocity();
	}

	inline static void CharacterControllerComponent_SetAngularVelocity(entt::entity entityId, glm::vec3* linearVelocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller actor found for entity {0}!", entityId);
			return;
		}

		actor->SetAngularVelocity(*linearVelocity);
	}

	inline static void CharacterControllerComponent_SetLinearVelocity(entt::entity entityId, glm::vec3* linearVelocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller actor found for entity {0}!", entityId);
			return;
		}

		actor->SetLinearVelocity(*linearVelocity);
	}
#pragma endregion

#pragma region BoxColliderComponent
	inline static void BoxColliderComponent_GetHalfSize(entt::entity entityId, glm::vec3* outHalfSize)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };

		if (entity.HasComponent<BoxColliderComponent>())
		{
			*outHalfSize = entity.GetComponent<BoxColliderComponent>().halfSize;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
			*outHalfSize = 0.f;
		}
	}

	inline static void BoxColliderComponent_SetHalfSize(entt::entity entityId, glm::vec3* halfSize)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			entity.GetComponent<BoxColliderComponent>().halfSize = *halfSize;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Box);
			if (!collPtr)
			{
				return;
			}

			Ref<BoxColliderShape> collider = std::reinterpret_pointer_cast<BoxColliderShape>(collPtr);
			collider->SetHalfSize(*halfSize);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
		}
	}

	inline static void BoxColliderComponent_GetOffset(entt::entity entityId, glm::vec3* outOffset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			*outOffset = entity.GetComponent<BoxColliderComponent>().offset;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
			*outOffset = 0.f;
		}
	}

	inline static void BoxColliderComponent_SetOffset(entt::entity entityId, glm::vec3* offset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			entity.GetComponent<BoxColliderComponent>().offset = *offset;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Box);
			if (!collPtr)
			{
				return;
			}

			Ref<BoxColliderShape> collider = std::reinterpret_pointer_cast<BoxColliderShape>(collPtr);
			collider->SetOffset(*offset);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
		}
	}

	inline static bool BoxColliderComponent_GetIsTrigger(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			return entity.GetComponent<BoxColliderComponent>().isTrigger;
		}

		VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
		return false;
	}

	inline static void BoxColliderComponent_SetIsTrigger(entt::entity entityId, bool* isTrigger)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			entity.GetComponent<BoxColliderComponent>().isTrigger = *isTrigger;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Box);
			if (!collPtr)
			{
				return;
			}

			Ref<BoxColliderShape> collider = std::reinterpret_pointer_cast<BoxColliderShape>(collPtr);
			collider->SetTrigger(*isTrigger);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
		}
	}
#pragma endregion

#pragma region SphereColliderComponent
	inline static float SphereColliderComponent_GetRadius(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };

		if (entity.HasComponent<SphereColliderComponent>())
		{
			return entity.GetComponent<SphereColliderComponent>().radius;
		}

		VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		return 0.f;
	}

	inline static void SphereColliderComponent_SetRadius(entt::entity entityId, float* radius)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			entity.GetComponent<SphereColliderComponent>().radius = *radius;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Sphere);
			if (!collPtr)
			{
				return;
			}

			Ref<SphereColliderShape> collider = std::reinterpret_pointer_cast<SphereColliderShape>(collPtr);
			collider->SetRadius(*radius);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		}
	}

	inline static void SphereColliderComponent_GetOffset(entt::entity entityId, glm::vec3* outOffset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			*outOffset = entity.GetComponent<SphereColliderComponent>().offset;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
			*outOffset = 0.f;
		}
	}

	inline static void SphereColliderComponent_SetOffset(entt::entity entityId, glm::vec3* offset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			entity.GetComponent<SphereColliderComponent>().offset = *offset;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Sphere);
			if (!collPtr)
			{
				return;
			}

			Ref<SphereColliderShape> collider = std::reinterpret_pointer_cast<SphereColliderShape>(collPtr);
			collider->SetOffset(*offset);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		}
	}

	inline static bool SphereColliderComponent_GetIsTrigger(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			return entity.GetComponent<SphereColliderComponent>().isTrigger;
		}

		VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		return false;
	}

	inline static void SphereColliderComponent_SetIsTrigger(entt::entity entityId, bool* isTrigger)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			entity.GetComponent<SphereColliderComponent>().isTrigger = *isTrigger;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Sphere);
			if (!collPtr)
			{
				return;
			}

			Ref<SphereColliderShape> collider = std::reinterpret_pointer_cast<SphereColliderShape>(collPtr);
			collider->SetTrigger(*isTrigger);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		}
	}
#pragma endregion

#pragma region CapsuleColliderComponent
	inline static float CapsuleColliderComponent_GetRadius(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			return entity.GetComponent<CapsuleColliderComponent>().radius;
		}

		VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		return 0.f;
	}

	inline static void CapsuleColliderComponent_SetRadius(entt::entity entityId, float* radius)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().radius = *radius;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Capsule);
			if (!collPtr)
			{
				return;
			}

			Ref<CapsuleColliderShape> collider = std::reinterpret_pointer_cast<CapsuleColliderShape>(collPtr);
			collider->SetRadius(*radius);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}

	inline static float CapsuleColliderComponent_GetHeight(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			return entity.GetComponent<CapsuleColliderComponent>().height;
		}

		VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		return 0.f;
	}

	inline static void CapsuleColliderComponent_SetHeight(entt::entity entityId, float* height)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().height = *height;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Capsule);
			if (!collPtr)
			{
				return;
			}

			Ref<CapsuleColliderShape> collider = std::reinterpret_pointer_cast<CapsuleColliderShape>(collPtr);
			collider->SetHeight(*height);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}

	inline static void CapsuleColliderComponent_GetOffset(entt::entity entityId, glm::vec3* outOffset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			*outOffset = entity.GetComponent<CapsuleColliderComponent>().offset;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
			*outOffset = 0.f;
		}
	}

	inline static void CapsuleColliderComponent_SetOffset(entt::entity entityId, glm::vec3* offset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().offset = *offset;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Capsule);
			if (!collPtr)
			{
				return;
			}

			Ref<CapsuleColliderShape> collider = std::reinterpret_pointer_cast<CapsuleColliderShape>(collPtr);
			collider->SetOffset(*offset);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}

	inline static bool CapsuleColliderComponent_GetIsTrigger(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			return entity.GetComponent<CapsuleColliderComponent>().isTrigger;
		}

		VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		return false;
	}

	inline static void CapsuleColliderComponent_SetIsTrigger(entt::entity entityId, bool* isTrigger)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().isTrigger = *isTrigger;

			auto actor = Physics::GetScene()->GetActor(entity);
			if (!actor)
			{
				return;
			}

			auto collPtr = actor->GetColliderOfType(ColliderType::Capsule);
			if (!collPtr)
			{
				return;
			}

			Ref<CapsuleColliderShape> collider = std::reinterpret_pointer_cast<CapsuleColliderShape>(collPtr);
			collider->SetTrigger(*isTrigger);
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}
#pragma endregion

#pragma region MeshColliderComponent
	inline static bool MeshColliderComponent_GetIsConvex(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<MeshColliderComponent>())
		{
			return entity.GetComponent<MeshColliderComponent>().isConvex;
		}

		VT_CORE_ERROR("Entity {0} does not have a MeshColliderComponent!", entityId);
		return false;
	}

	inline static void MeshColliderComponent_SetIsConvex(entt::entity entityId, bool* isConvex)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<MeshColliderComponent>())
		{
			entity.GetComponent<MeshColliderComponent>().isConvex = *isConvex;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a MeshColliderComponent!", entityId);
		}
	}

	inline static bool MeshColliderComponent_GetIsTrigger(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<MeshColliderComponent>())
		{
			return entity.GetComponent<MeshColliderComponent>().isTrigger;
		}

		VT_CORE_ERROR("Entity {0} does not have a MeshColliderComponent!", entityId);
		return false;
	}

	inline static void MeshColliderComponent_SetIsTrigger(entt::entity entityId, bool* isTrigger)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<MeshColliderComponent>())
		{
			entity.GetComponent<MeshColliderComponent>().isTrigger = *isTrigger;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a MeshColliderComponent!", entityId);
		}
	}

	inline static int32_t MeshColliderComponent_GetSubMeshIndex(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<MeshColliderComponent>())
		{
			return entity.GetComponent<MeshColliderComponent>().subMeshIndex;
		}

		VT_CORE_ERROR("Entity {0} does not have a MeshColliderComponent!", entityId);
		return -1;
	}

	inline static void MeshColliderComponent_SetSubMeshIndex(entt::entity entityId, int32_t* subMeshIndex)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<MeshColliderComponent>())
		{
			entity.GetComponent<MeshColliderComponent>().subMeshIndex = *subMeshIndex;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a MeshColliderComponent!", entityId);
		}
	}

	inline static uint64_t MeshColliderComponent_GetColliderMesh(entt::entity entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<MeshColliderComponent>())
		{
			return Asset::Null();
		}

		return entity.GetComponent<MeshColliderComponent>().colliderMesh;
	}

	inline static void MeshColliderComponent_SetColliderMesh(entt::entity entityId, uint64_t meshHandle)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<MeshColliderComponent>())
		{
			return;
		}

		entity.GetComponent<MeshColliderComponent>().colliderMesh = meshHandle;
	}
#pragma endregion

#pragma region InputMapper

	inline static int InputMapper_GetKey(MonoString* key)
	{
		const auto keyStr = MonoScriptUtils::GetStringFromMonoString(key);
		return InputMapper::GetKey(keyStr);
	}

	inline static void InputMapper_SetKey(MonoString* key, int value)
	{
		const auto keyStr = MonoScriptUtils::GetStringFromMonoString(key);
		InputMapper::SetKey(keyStr, value);
	}

	inline static void InputMapper_ResetKey(MonoString* key)
	{
		const auto keyStr = MonoScriptUtils::GetStringFromMonoString(key);
		InputMapper::ResetKey(keyStr);
	}

#pragma endregion

#pragma region Input

	inline static bool Input_KeyPressed(int32_t keyCode)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsKeyPressed(keyCode);
	}

	inline static MonoArray* Input_GetAllKeyPressed()
	{
		std::vector<int> tempKeyVec = Input::GetAllKeyPressed();

		MonoArray* array = mono_array_new(MonoScriptEngine::GetAppDomain(), MonoScriptEngine::GetEntityClass()->GetClass(), tempKeyVec.size());

		for (uint32_t i = 0; const auto & key : tempKeyVec)
		{
			mono_array_set(array, int, i, key);
			i++;
		}

		return array;
	}

	inline static bool Input_KeyReleased(int32_t keyCode)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsKeyReleased(keyCode);
	}

	inline static bool Input_MousePressed(int32_t button)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsMouseButtonPressed(button);
	}

	inline static bool Input_MouseReleased(int32_t button)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsMouseButtonReleased(button);
	}

	inline static bool Input_KeyDown(int32_t keyCode)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsKeyDown(keyCode);
	}

	inline static bool Input_KeyUp(int32_t keyCode)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsKeyUp(keyCode);
	}

	inline static bool Input_MouseDown(int32_t button)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsMouseButtonDown(button);
	}

	inline static bool Input_MouseUp(int32_t button)
	{
		if (Input::IsInputDisabled())
		{
			return false;
		}
		return Input::IsMouseButtonUp(button);
	}

	inline static void Input_SetMousePosition(float x, float y)
	{
		if (Input::IsInputDisabled())
		{
			return;
		}
		Input::SetMousePosition(x, y);
	}

	inline static void Input_GetMousePosition(glm::vec2* position)
	{
		if (Input::IsInputDisabled())
		{
			return;
		}

		if (Application::Get().IsRuntime())
		{
			auto [wx, wy] = Application::Get().GetWindow().GetPosition();
			auto [x, y] = Input::GetMousePosition();
			*position = { x - wx, y - wy };
		}
		else
		{
			const auto viewportPos = Input::GetViewportMousePosition();
			*position = viewportPos;
		}
	}

	inline static void Input_ShowCursor(bool state)
	{
		Volt::Input::ShowCursor(state);
	}

	inline static float Input_GetScrollOffset()
	{
		return Volt::Input::GetScrollOffset();
	}
#pragma endregion

#pragma region PhysicsActor
	inline static void PhysicsActor_SetKinematicTarget(entt::entity entityId, glm::vec3* position, glm::quat* rotation)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->SetKinematicTarget(*position, glm::eulerAngles(*rotation));
	}

	inline static void PhysicsActor_SetLinearVelocity(entt::entity entityId, glm::vec3* velocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->SetLinearVelocity(*velocity);
	}

	inline static void PhysicsActor_SetAngularVelocity(entt::entity entityId, glm::vec3* velocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->SetAngularVelocity(*velocity);
	}

	inline static void PhysicsActor_GetLinearVelocity(entt::entity entityId, glm::vec3* velocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		*velocity = actor->GetLinearVelocity();
	}

	inline static void PhysicsActor_GetAngularVelocity(entt::entity entityId, glm::vec3* velocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		*velocity = actor->GetAngularVelocity();
	}

	inline static void PhysicsActor_SetMaxLinearVelocity(entt::entity entityId, float* velocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->SetMaxLinearVelocity(*velocity);
	}

	inline static void PhysicsActor_SetMaxAngularVelocity(entt::entity entityId, float* velocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->SetMaxAngularVelocity(*velocity);
	}

	inline static void PhysicsActor_GetKinematicTargetPosition(entt::entity entityId, glm::vec3* outPosition)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		*outPosition = actor->GetKinematicTargetPosition();
	}

	inline static void PhysicsActor_GetKinematicTargetRotation(entt::entity entityId, glm::quat* outRotation)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		*outRotation = actor->GetKinematicTargetRotation();
	}

	inline static void PhysicsActor_AddForce(entt::entity entityId, glm::vec3* force, ForceMode forceMode)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->AddForce(*force, forceMode);
	}

	inline static void PhysicsActor_AddTorque(entt::entity entityId, glm::vec3* torque, ForceMode forceMode)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->AddTorque(*torque, forceMode);
	}

	inline static void PhysicsActor_WakeUp(entt::entity entityId)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->WakeUp();
	}

	inline static void PhysicsActor_PutToSleep(entt::entity entityId)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid actor found for entity {0}!", entityId);
			return;
		}

		actor->PutToSleep();
	}
#pragma endregion

#pragma region PhysicsControllerActor
	inline static float PhysicsControllerActor_GetHeight(entt::entity entityId)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return 0.f;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return 0.f;
		}

		return actor->GetHeight();
	}

	inline static void PhysicsControllerActor_SetHeight(entt::entity entityId, float height)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		actor->SetHeight(height);
	}

	inline static float PhysicsControllerActor_GetRadius(entt::entity entityId)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return 0.f;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return 0.f;
		}

		return actor->GetRadius();
	}

	inline static void PhysicsControllerActor_SetRadius(entt::entity entityId, float radius)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		actor->SetRadius(radius);
	}

	inline static void PhysicsControllerActor_Move(entt::entity entityId, glm::vec3* velocity)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		actor->Move(*velocity);
	}

	inline static void PhysicsControllerActor_SetPosition(entt::entity entityId, glm::vec3* position)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		actor->SetPosition(*position);
	}

	inline static void PhysicsControllerActor_SetFootPosition(entt::entity entityId, glm::vec3* position)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		actor->SetFootPosition(*position);
	}

	inline static void PhysicsControllerActor_GetPosition(entt::entity entityId, glm::vec3* position)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		*position = actor->GetPosition();
	}

	inline static void PhysicsControllerActor_GetFootPosition(entt::entity entityId, glm::vec3* position)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		*position = actor->GetFootPosition();
	}

	inline static bool PhysicsControllerActor_IsGrounded(entt::entity entityId)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return false;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return false;
		}

		return actor->IsGrounded();
	}

	inline static void PhysicsControllerActor_Jump(entt::entity entityId, float jumpForce)
	{
		auto physicsScene = Physics::GetScene();
		if (!physicsScene)
		{
			VT_CORE_ERROR("No valid physics scene found!");
			return;
		}

		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto actor = physicsScene->GetControllerActor(entity);
		if (!actor)
		{
			VT_CORE_ERROR("No valid controller found for entity {0}!", entityId);
			return;
		}

		actor->Jump(jumpForce);
	}
#pragma endregion

#pragma region Physics
	inline static void Physics_GetGravity(glm::vec3* outGravity)
	{
		*outGravity = Physics::GetScene()->GetGravity();
	}

	inline static void Physics_SetGravity(glm::vec3* gravity)
	{
		Physics::GetScene()->SetGravity(*gravity);
	}

	inline static bool Physics_Raycast(glm::vec3* origin, glm::vec3* direction, RaycastHit* outHit, float maxDistance)
	{
		return Physics::GetScene()->Raycast(*origin, *direction, maxDistance, outHit);
	}

	inline static bool Physics_RaycastLayerMask(glm::vec3* origin, glm::vec3* direction, RaycastHit* outHit, float maxDistance, uint32_t layerMask)
	{
		return Physics::GetScene()->Raycast(*origin, *direction, maxDistance, outHit, layerMask);
	}

	inline static MonoArray* Physics_OverlapBox(glm::vec3* origin, glm::vec3* halfSize, uint32_t layerMask)
	{
		std::vector<Entity> tempHitList;
		bool hasHit = Physics::GetScene()->OverlapBox(*origin, *halfSize, tempHitList, layerMask);

		if (!hasHit) return nullptr;

		MonoArray* array = mono_array_new(MonoScriptEngine::GetAppDomain(), MonoScriptEngine::GetEntityClass()->GetClass(), tempHitList.size());

		for (uint32_t i = 0; const auto & hit : tempHitList)
		{
			auto instance = MonoScriptEngine::GetEntityFromId(hit.GetID());
			if (!instance)
			{
				instance = MonoScriptEngine::GetOrCreateMonoEntity(hit.GetID());
			}

			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			mono_array_set(array, MonoObject*, i, monoObject);
			i++;
		}

		return array;
	}

	inline static MonoArray* Physics_OverlapSphere(glm::vec3* origin, float radius, uint32_t layerMask)
	{
		std::vector<Entity> tempHitList;
		bool hasHit = Physics::GetScene()->OverlapSphere(*origin, radius, tempHitList, layerMask);

		if (!hasHit) return nullptr;

		MonoArray* array = mono_array_new(MonoScriptEngine::GetAppDomain(), MonoScriptEngine::GetEntityClass()->GetClass(), tempHitList.size());

		for (uint32_t i = 0; const auto & hit : tempHitList)
		{
			auto instance = MonoScriptEngine::GetEntityFromId(hit.GetID());
			if (!instance)
			{
				instance = MonoScriptEngine::GetOrCreateMonoEntity(hit.GetID());
			}

			auto monoObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
			mono_array_set(array, MonoObject*, i, monoObject);
			i++;
		}

		return array;
	}
#pragma endregion

#pragma region Noise

	inline static void Noise_SetFrequency(float frequency)
	{
		Noise::SetFrequency(frequency);
	}

	inline static void Noise_SetSeed(int32_t seed)
	{
		Noise::SetSeed(seed);
	}

	inline static float Noise_Perlin(float x, float y, float z)
	{
		return Noise::GetNoise(x, y, z);
	}

#pragma endregion 

#pragma region AMP

	inline static bool AMP_PlayOneshotEvent(MonoString* aEventName)
	{
		std::string eventName = MonoScriptUtils::GetStringFromMonoString(aEventName);

		return Amp::WwiseAudioManager::PlayOneShotEvent(eventName.c_str());
	}

	inline static bool AMP_SetRTPC(MonoString* aRTPC, int aValue)
	{
		std::string RTPCName = MonoScriptUtils::GetStringFromMonoString(aRTPC);

		return Amp::WwiseAudioManager::SetRTPC(RTPCName.c_str(), static_cast<float>(aValue));
	}

	inline static void AMP_StopAllEvents()
	{
		Amp::WwiseAudioManager::StopAllEvents();
	}


	//EVENT
	inline static uint32_t AudioSourceComponent_PlayEvent(entt::entity entityId, MonoString* aEventName)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		std::string eventName = MonoScriptUtils::GetStringFromMonoString(aEventName);
		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return 0;
		}

		uint32_t playingID = 0;
		entity.GetComponent<AudioSourceComponent>().PlayEvent(eventName.c_str(), playingID);

		return playingID;
	}

	inline static bool AudioSourceComponent_PlayOneshotEvent(entt::entity entityId, MonoString* aEventName)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		std::string eventName = MonoScriptUtils::GetStringFromMonoString(aEventName);
		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return 0;
		}

		return 	entity.GetComponent<AudioSourceComponent>().PlayOneshotEvent(eventName.c_str(), entity.GetPosition(), entity.GetForward(), entity.GetUp());
	}

	inline static bool AudioSourceComponent_StopEvent(entt::entity entityId, uint32_t aPlayingID)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return false;
		}

		return entity.GetComponent<AudioSourceComponent>().StopEvent(aPlayingID);
	}

	inline static bool AudioSourceComponent_PauseEvent(entt::entity entityId, uint32_t aPlayingID)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return false;
		}

		return entity.GetComponent<AudioSourceComponent>().PauseEvent(aPlayingID);
	}

	inline static bool AudioSourceComponent_ResumeEvent(entt::entity entityId, uint32_t aPlayingID)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return false;
		}

		return entity.GetComponent<AudioSourceComponent>().ResumeEvent(aPlayingID);
	}

	inline static void AudioSourceComponent_StopAllEvents(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return;
		}

		entity.GetComponent<AudioSourceComponent>().StopEvent(entityId);
	}

	//GAME SYNCS

	inline static bool AudioSourceComponent_SetState(entt::entity entityId, MonoString* aStateGroup, MonoString* aState)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return false;
		}

		std::string stateGroup = MonoScriptUtils::GetStringFromMonoString(aStateGroup);
		std::string stateName = MonoScriptUtils::GetStringFromMonoString(aState);

		return entity.GetComponent<AudioSourceComponent>().SetState(stateGroup.c_str(), stateName.c_str());
	}

	inline static bool AudioSourceComponent_SetSwitch(entt::entity entityId, MonoString* aSwitchGroup, MonoString* aState)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return false;
		}

		std::string switchGroup = MonoScriptUtils::GetStringFromMonoString(aSwitchGroup);
		std::string stateName = MonoScriptUtils::GetStringFromMonoString(aState);

		return entity.GetComponent<AudioSourceComponent>().SetSwitch(switchGroup.c_str(), stateName.c_str());
	}

	inline static bool AudioSourceComponent_SetParameter(entt::entity entityId, MonoString* aParameterName, float aValue)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return false;
		}

		std::string parameterName = MonoScriptUtils::GetStringFromMonoString(aParameterName);

		return entity.GetComponent<AudioSourceComponent>().SetParameter(parameterName.c_str(), aValue);
	}

	inline static bool AudioSourceComponent_SetParameterOverTime(entt::entity entityId, MonoString* aParameterName, float aValue, uint32_t aOvertime)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<AudioSourceComponent>())
		{
			return false;
		}

		std::string parameterName = MonoScriptUtils::GetStringFromMonoString(aParameterName);

		return entity.GetComponent<AudioSourceComponent>().SetParameter(parameterName.c_str(), aValue, aOvertime);
	}


#pragma endregion 

#pragma region Vision

	inline static void Vision_SetActiveCamera(entt::entity entityId)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetActiveCamera(Volt::Entity{ entityId, MonoScriptEngine::GetSceneContext() });
	}

	inline static entt::entity Vision_GetActiveCamera()
	{
		const Volt::Entity ent = MonoScriptEngine::GetSceneContext()->GetVision().GetActiveCamera();
		if (ent)
		{
			return ent.GetID();
		}
		return 0;
	}

	inline static void Vision_DoCameraShake(entt::entity entityId, Volt::CameraShakeSettings* shakeSettings)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().DoCameraShake(Volt::Entity{ entityId, MonoScriptEngine::GetSceneContext() }, *shakeSettings);
	}

	inline static void Vision_SetCameraFollow(entt::entity cameraId, entt::entity followId)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetCameraFollow(Volt::Entity{ cameraId, MonoScriptEngine::GetSceneContext() }, Volt::Entity{ followId, MonoScriptEngine::GetSceneContext() });
	}

	inline static void Vision_SetCameraLookAt(entt::entity cameraId, entt::entity followId)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetCameraLookAt(Volt::Entity{ cameraId, MonoScriptEngine::GetSceneContext() }, Volt::Entity{ followId, MonoScriptEngine::GetSceneContext() });
	}

	inline static void Vision_SetCameraFocusPoint(entt::entity cameraId, entt::entity focusId)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetCameraFocusPoint(Volt::Entity{ cameraId, MonoScriptEngine::GetSceneContext() }, Volt::Entity{ focusId, MonoScriptEngine::GetSceneContext() });
	}

	inline static void Vision_SetCameraDampAmount(entt::entity cameraId, float dampAmount)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetCameraDampAmount(Volt::Entity{ cameraId, MonoScriptEngine::GetSceneContext() }, dampAmount);
	}

	inline static void Vision_SetCameraFieldOfView(entt::entity cameraId, float aFov)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetCameraFieldOfView(Volt::Entity{ cameraId, MonoScriptEngine::GetSceneContext() }, aFov);
	}

	inline static void Vision_SetCameraLocked(entt::entity cameraId, bool locked)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetCameraLocked(Volt::Entity{ cameraId, MonoScriptEngine::GetSceneContext() }, locked);
	}

	inline static void Vision_SetCameraMouseSensentivity(entt::entity cameraId, float mouseSens)
	{
		MonoScriptEngine::GetSceneContext()->GetVision().SetCameraMouseSensentivity(Volt::Entity{ cameraId, MonoScriptEngine::GetSceneContext() }, mouseSens);
	}

#pragma endregion

#pragma region Render
	inline static void DebugRenderer_DrawLine(glm::vec3* startPosition, glm::vec3* endPosition, glm::vec4* color)
	{
		DebugRenderer::DrawLine(*startPosition, *endPosition, *color);
	}

	inline static void DebugRenderer_DrawSprite(glm::vec3* position, glm::vec3* rotation, glm::vec3* scale, glm::vec4* color)
	{
		DebugRenderer::DrawSprite(*position, *rotation, *scale, *color);
	}

	inline static void DebugRenderer_DrawText(MonoString* text, glm::vec3* position, glm::vec3* rotation, glm::vec3* scale)
	{
		const auto str = MonoScriptUtils::GetStringFromMonoString(text);
		DebugRenderer::DrawText(str, *position, *rotation, *scale);
	}
#pragma endregion 

#pragma region Navigation

	inline static MonoArray* Navigation_FindPath(glm::vec3* start, glm::vec3* end, glm::vec3* searchDistance)
	{
		auto navmesh = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh();

		MonoArray* array = nullptr;

		if (navmesh)
		{
			auto path = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh()->GetNavMesh()->FindPath(*start, *end, *searchDistance);

			MonoClass* vec3Class = mono_class_from_name(MonoScriptEngine::GetCoreAssembly().assemblyImage, "Volt", "Vector3");

			auto& coreAssembly = MonoScriptEngine::GetCoreAssembly();
			array = mono_array_new(coreAssembly.domain, vec3Class, path.size());

			for (uint16_t i = 0; i < path.size(); ++i)
			{
				mono_array_set(array, glm::vec3, i, path[i]);
			}
		}

		return array;
	}

#pragma endregion

#pragma region NavAgentComponent

	inline static void NavAgentComponent_GetTarget(entt::entity entityId, glm::vec3* position)
	{
		auto navmesh = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh();

		if (navmesh)
		{
			*position = *(glm::vec3*)&Volt::Application::Get().GetNavigationSystem().GetVTNavMesh()->GetCrowd()->GetAgent(entityId)->targetPos;
		}
	}

	inline static void NavAgentComponent_SetTarget(entt::entity entityId, glm::vec3* position)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto navmesh = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh();

		if (navmesh)
		{
			Volt::Application::Get().GetNavigationSystem().GetVTNavMesh()->GetCrowd()->SetAgentTarget(entity, *position);
		}
	}

	inline static void NavAgentComponent_GetPosition(entt::entity entityId, glm::vec3* position)
	{
		auto navmesh = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh();

		if (navmesh)
		{
			*position = *(glm::vec3*)&Volt::Application::Get().GetNavigationSystem().GetVTNavMesh()->GetCrowd()->GetAgent(entityId)->npos;
		}
	}

	inline static void NavAgentComponent_SetPosition(entt::entity entityId, glm::vec3* position)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		auto navmesh = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh();

		if (navmesh)
		{
			Volt::Application::Get().GetNavigationSystem().GetVTNavMesh()->GetCrowd()->SetAgentPosition(entity, *position);
		}
	}

	inline static void NavAgentComponent_GetVelocity(entt::entity entityId, glm::vec3* velocity)
	{
		auto navmesh = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh();

		if (navmesh)
		{
			*velocity = *(glm::vec3*)&Volt::Application::Get().GetNavigationSystem().GetVTNavMesh()->GetCrowd()->GetAgent(entityId)->vel;
		}
	}

	inline static void NavAgentComponent_UpdateParams(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		auto navmesh = Volt::Application::Get().GetNavigationSystem().GetVTNavMesh();

		if (navmesh)
		{
			Volt::Application::Get().GetNavigationSystem().GetVTNavMesh()->GetCrowd()->UpdateAgentParams(entity);
		}
	}

	inline static bool NavAgentComponent_GetActive(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return 0.f;
		}

		return entity.GetComponent<NavAgentComponent>().active;
	}

	inline static void NavAgentComponent_SetActive(entt::entity entityId, bool value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		entity.GetComponent<NavAgentComponent>().active = value;
		NavAgentComponent_UpdateParams(entityId);

		if (value)
		{
			auto position = entity.GetPosition();
			NavAgentComponent_SetPosition(entityId, &position);
		}
	}

	inline static float NavAgentComponent_GetRadius(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return 0.f;
		}

		return entity.GetComponent<NavAgentComponent>().radius;
	}

	inline static void NavAgentComponent_SetRadius(entt::entity entityId, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		entity.GetComponent<NavAgentComponent>().radius = value;
		NavAgentComponent_UpdateParams(entityId);
	}

	inline static float NavAgentComponent_GetHeight(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return 0.f;
		}

		return entity.GetComponent<NavAgentComponent>().height;
	}

	inline static void NavAgentComponent_SetHeight(entt::entity entityId, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		entity.GetComponent<NavAgentComponent>().height = value;
		NavAgentComponent_UpdateParams(entityId);
	}

	inline static float NavAgentComponent_GetMaxSpeed(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return 0.f;
		}

		return entity.GetComponent<NavAgentComponent>().maxSpeed;
	}

	inline static void NavAgentComponent_SetMaxSpeed(entt::entity entityId, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		entity.GetComponent<NavAgentComponent>().maxSpeed = value;
		NavAgentComponent_UpdateParams(entityId);
	}

	inline static float NavAgentComponent_GetAcceleration(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return 0.f;
		}

		return entity.GetComponent<NavAgentComponent>().acceleration;
	}

	inline static void NavAgentComponent_SetAcceleration(entt::entity entityId, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		entity.GetComponent<NavAgentComponent>().acceleration = value;
		NavAgentComponent_UpdateParams(entityId);
	}

	inline static float NavAgentComponent_GetSeperationWeight(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return 0.f;
		}

		return entity.GetComponent<NavAgentComponent>().separationWeight;
	}

	inline static void NavAgentComponent_SetSeperationWeight(entt::entity entityId, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		entity.GetComponent<NavAgentComponent>().separationWeight = value;
		NavAgentComponent_UpdateParams(entityId);
	}

	inline static ObstacleAvoidanceQuality NavAgentComponent_GetObstacleAvoidanceQuality(entt::entity entityId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return ObstacleAvoidanceQuality::None;
		}

		return entity.GetComponent<NavAgentComponent>().obstacleAvoidanceQuality;
	}

	inline static void NavAgentComponent_SetObstacleAvoidanceQuality(entt::entity entityId, ObstacleAvoidanceQuality* value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (!entity.HasComponent<NavAgentComponent>())
		{
			return;
		}

		entity.GetComponent<NavAgentComponent>().obstacleAvoidanceQuality = *value;
		NavAgentComponent_UpdateParams(entityId);
	}

#pragma endregion

#pragma region VisualScriptingComponent
	inline static void VisualScriptingComponent_SetParameterFloat(entt::entity id, MonoString* name, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<VisualScriptingComponent>())
		{
			return;
		}

		if (!entity.GetComponent<VisualScriptingComponent>().graph)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto graph = entity.GetComponent<VisualScriptingComponent>().graph;
		graph->SetParameterValue(paramName, value);
	}

	inline static void VisualScriptingComponent_SetParameterInt(entt::entity id, MonoString* name, int32_t value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<VisualScriptingComponent>())
		{
			return;
		}

		if (!entity.GetComponent<VisualScriptingComponent>().graph)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto graph = entity.GetComponent<VisualScriptingComponent>().graph;
		graph->SetParameterValue(paramName, value);
	}

	inline static void VisualScriptingComponent_SetParameterBool(entt::entity id, MonoString* name, bool value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<VisualScriptingComponent>())
		{
			return;
		}

		if (!entity.GetComponent<VisualScriptingComponent>().graph)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto graph = entity.GetComponent<VisualScriptingComponent>().graph;
		graph->SetParameterValue(paramName, value);
	}

	inline static void VisualScriptingComponent_SetParameterVector3(entt::entity id, MonoString* name, glm::vec3* value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<VisualScriptingComponent>())
		{
			return;
		}

		if (!entity.GetComponent<VisualScriptingComponent>().graph)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto graph = entity.GetComponent<VisualScriptingComponent>().graph;
		graph->SetParameterValue(paramName, value);
	}

	inline static void VisualScriptingComponent_SetParameterString(entt::entity id, MonoString* name, MonoString* value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<VisualScriptingComponent>())
		{
			return;
		}

		if (!entity.GetComponent<VisualScriptingComponent>().graph)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto graph = entity.GetComponent<VisualScriptingComponent>().graph;
		graph->SetParameterValue(paramName, value);
	}
#pragma endregion

#pragma region AnimationControllerComponent

	inline static void AnimationControllerComponent_SetParameterFloat(entt::entity id, MonoString* name, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		controller->GetGraph()->SetParameterValue(paramName, value);
	}

	inline static void AnimationControllerComponent_SetParameterInt(entt::entity id, MonoString* name, int32_t value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		controller->GetGraph()->SetParameterValue(paramName, value);
	}

	inline static void AnimationControllerComponent_SetParameterBool(entt::entity id, MonoString* name, bool value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		controller->GetGraph()->SetParameterValue(paramName, value);
	}

	inline static void AnimationControllerComponent_SetParameterVector3(entt::entity id, MonoString* name, glm::vec3* value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		controller->GetGraph()->SetParameterValue(paramName, value);
	}

	inline static void AnimationControllerComponent_SetParameterString(entt::entity id, MonoString* name, MonoString* value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		controller->GetGraph()->SetParameterValue(paramName, value);
	}

	inline static void AnimationControllerComponent_GetBoundingSphere(entt::entity id, glm::vec3* center, float* radius)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		auto animatedCharacter = AssetManager::GetAsset<AnimatedCharacter>(controller->GetGraph()->GetCharacterHandle());
		*center = animatedCharacter->GetSkin()->GetBoundingSphere().center;
		*radius = animatedCharacter->GetSkin()->GetBoundingSphere().radius;
	}

	inline static float AnimationControllerComponent_GetParameterFloat(entt::entity id, MonoString* name)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0.0f;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return 0.0f;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return 0.0f;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		return controller->GetGraph()->GetParameterValue<float>(paramName);
	}

	inline static int AnimationControllerComponent_GetParameterInt(entt::entity id, MonoString* name)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return 0;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return 0;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		return controller->GetGraph()->GetParameterValue<int>(paramName);
	}

	inline static bool AnimationControllerComponent_GetParameterBool(entt::entity id, MonoString* name)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return 0;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return 0;
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		return controller->GetGraph()->GetParameterValue<bool>(paramName);
	}

	inline static glm::vec3 AnimationControllerComponent_GetParameterVector3(entt::entity id, MonoString* name)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return { 0,0,0 };
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return { 0,0,0 };
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return { 0,0,0 };
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		return controller->GetGraph()->GetParameterValue<glm::vec3>(paramName);
	}

	inline static std::string AnimationControllerComponent_GetParameterString(entt::entity id, MonoString* name)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return "Null";
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return "Null";
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return "Null";
		}

		const auto paramName = MonoScriptUtils::GetStringFromMonoString(name);
		auto controller = entity.GetComponent<AnimationControllerComponent>().controller;
		return controller->GetGraph()->GetParameterValue<std::string>(paramName);
	}

	inline static void AnimationControllerComponent_GetRootMotion(entt::entity id, glm::vec3* value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		*value = entity.GetComponent<AnimationControllerComponent>().controller->GetRootMotion().position;
	}

	inline static void AnimationControllerComponent_AttachEntity(MonoString* attachmentName, entt::entity id, entt::entity attachId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		const std::string str = MonoScriptUtils::GetStringFromMonoString(attachmentName);
		Entity attachEntity{ attachId, scene };

		entity.GetComponent<AnimationControllerComponent>().controller->AttachEntity(str, attachEntity);
	}

	inline static void AnimationControllerComponent_DetachEntity(entt::entity id, entt::entity attachId)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		if (!entity.GetComponent<AnimationControllerComponent>().controller)
		{
			return;
		}

		Entity attachEntity{ attachId, scene };
		entity.GetComponent<AnimationControllerComponent>().controller->DetachEntity(attachEntity);
	}

	inline static bool AnimationControllerComponent_HasOverrideMaterial(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return false;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return false;
		}

		return entity.GetComponent<AnimationControllerComponent>().overrideMaterial != Asset::Null();
	}

	inline static void AnimationControllerComponent_SetOverrideMaterial(entt::entity id, uint64_t materialHandle)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		entity.GetComponent<AnimationControllerComponent>().overrideMaterial = materialHandle;
	}

	inline static uint64_t AnimationControllerComponent_GetOverrideMaterial(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return 0;
		}

		return entity.GetComponent<AnimationControllerComponent>().overrideMaterial;
	}

	inline static void AnimationControllerComponent_SetOverrideSkin(entt::entity id, uint64_t skinHandle)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		entity.GetComponent<AnimationControllerComponent>().overrideSkin = skinHandle;
	}

	inline static uint64_t AnimationControllerComponent_GetOverrideSkin(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return 0;
		}

		return entity.GetComponent<AnimationControllerComponent>().overrideSkin;
	}

	inline static void AnimationControllerComponent_SetController(entt::entity id, uint64_t animGraphHandle)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<AnimationControllerComponent>())
		{
			return;
		}

		auto graph = AssetManager::GetAsset<AnimationGraphAsset>(animGraphHandle);
		if (graph && graph->IsValid())
		{
			entity.GetComponent<AnimationControllerComponent>().controller = CreateRef<AnimationController>(graph, entity);
		}
	}
#pragma endregion

#pragma region TextRendererComponent
	inline static void TextRendererComponent_GetText(entt::entity id, MonoString* outText)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TextRendererComponent>())
		{
			return;
		}

		outText = MonoScriptUtils::GetMonoStringFromString(entity.GetComponent<TextRendererComponent>().text);
	}

	inline static void TextRendererComponent_SetText(entt::entity id, MonoString* text)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TextRendererComponent>())
		{
			return;
		}

		entity.GetComponent<TextRendererComponent>().text = MonoScriptUtils::GetStringFromMonoString(text);
	}

	inline static float TextRendererComponent_GetMaxWidth(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0.f;
		}

		if (!entity.HasComponent<TextRendererComponent>())
		{
			return 0.f;
		}

		return entity.GetComponent<TextRendererComponent>().maxWidth;
	}

	inline static void TextRendererComponent_SetMaxWidth(entt::entity id, float value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TextRendererComponent>())
		{
			return;
		}

		entity.GetComponent<TextRendererComponent>().maxWidth = value;
	}

	inline static void TextRendererComponent_GetColor(entt::entity id, glm::vec4* outColor)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TextRendererComponent>())
		{
			return;
		}

		*outColor = entity.GetComponent<TextRendererComponent>().color;
	}

	inline static void TextRendererComponent_SetColor(entt::entity id, glm::vec4* value)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<TextRendererComponent>())
		{
			return;
		}

		entity.GetComponent<TextRendererComponent>().color = *value;
	}
#pragma endregion

#pragma region Timer
	inline static float Time_GetDeltaTime()
	{
		return MonoScriptEngine::GetSceneContext()->GetDeltaTime();
	}

	inline static void Time_SetTimeScale(float timeScale)
	{
		MonoScriptEngine::GetSceneContext()->SetTimeScale(timeScale);
	}
#pragma endregion

#pragma region Project
	inline static MonoString* Project_GetDirectory()
	{
		std::string directory = ProjectManager::GetDirectory().string();
		return MonoScriptUtils::GetMonoStringFromString(directory);
	}

	inline static MonoString* Project_GetProjectName()
	{
		std::string name = ProjectManager::GetProject().name;
		return MonoScriptUtils::GetMonoStringFromString(name);
	}

	inline static MonoString* Project_GetCompanyName()
	{
		std::string name = ProjectManager::GetProject().companyName;
		return MonoScriptUtils::GetMonoStringFromString(name);
	}
#pragma endregion

#pragma region MeshComponent
	inline static uint64_t MeshComponent_GetMeshHandle(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<MeshComponent>())
		{
			return 0;
		}

		return entity.GetComponent<MeshComponent>().handle;
	}

	inline static void MeshComponent_SetMeshHandle(entt::entity id, AssetHandle handle)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity.HasComponent<MeshComponent>())
		{
			return;
		}

		entity.GetComponent<MeshComponent>().handle = handle;
	}

	inline static bool MeshComponent_HasOverrideMaterial(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return false;
		}

		if (!entity.HasComponent<MeshComponent>())
		{
			return false;
		}

		return entity.GetComponent<MeshComponent>().overrideMaterial != Asset::Null();
	}

	inline static void MeshComponent_SetOverrideMaterial(entt::entity id, uint64_t materialHandle)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<MeshComponent>())
		{
			return;
		}

		entity.GetComponent<MeshComponent>().overrideMaterial = materialHandle;
	}

	inline static uint64_t MeshComponent_GetOverrideMaterial(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<MeshComponent>())
		{
			return 0;
		}

		return entity.GetComponent<MeshComponent>().overrideMaterial;
	}
#pragma endregion 

#pragma region SpotlightComponent

	inline static void SpotlightComponent_GetColor(entt::entity id, glm::vec3* outColor)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<SpotLightComponent>())
		{
			return;
		}

		*outColor = entity.GetComponent<SpotLightComponent>().color;
	}

	inline static void SpotlightComponent_SetColor(entt::entity id, glm::vec3* color)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<SpotLightComponent>())
		{
			return;
		}

		entity.GetComponent<SpotLightComponent>().color = *color;
	}

	inline static bool SpotlightComponent_GetIntensity(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<SpotLightComponent>())
		{
			return 0;
		}

		float intens = entity.GetComponent<SpotLightComponent>().intensity;

		return intens;
	}

	inline static void SpotlightComponent_SetIntensity(entt::entity id, float intensity)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<SpotLightComponent>())
		{
			return;
		}

		entity.GetComponent<SpotLightComponent>().intensity = intensity;
	}

#pragma endregion 


#pragma region PointlightComponent

	inline static void PointlightComponent_GetColor(entt::entity id, glm::vec3* outColor)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<PointLightComponent>())
		{
			return;
		}

		*outColor = entity.GetComponent<PointLightComponent>().color;
	}

	inline static void PointlightComponent_SetColor(entt::entity id, glm::vec3* color)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<PointLightComponent>())
		{
			return;
		}

		entity.GetComponent<PointLightComponent>().color = *color;
	}

	inline static bool PointlightComponent_GetIntensity(entt::entity id)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return 0;
		}

		if (!entity.HasComponent<PointLightComponent>())
		{
			return 0;
		}

		return entity.GetComponent<PointLightComponent>().intensity;
	}

	inline static void PointlightComponent_SetIntensity(entt::entity id, float intensity)
	{
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ id, scene };

		if (!entity)
		{
			return;
		}

		if (!entity.HasComponent<PointLightComponent>())
		{
			return;
		}

		entity.GetComponent<PointLightComponent>().intensity = intensity;
	}

#pragma endregion 


#pragma region Mesh
	inline static uint64_t Mesh_GetMaterial(uint64_t meshHandle)
	{
		Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
		if (!mesh || !mesh->IsValid())
		{
			return 0;
		}

		return mesh->GetMaterial()->handle;
	}
#pragma endregion

#pragma region Material
	inline static void Material_SetColor(uint64_t handle, glm::vec4* color)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		material->GetSubMaterialAt(0)->GetMaterialData().color = *color;
		Renderer::UpdateMaterial(material->GetSubMaterialAt(0).get());
	}

	inline static void Material_GetColor(uint64_t handle, glm::vec4* outColor)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		*outColor = material->GetSubMaterialAt(0)->GetMaterialData().color;
	}

	inline static void Material_SetEmissiveColor(uint64_t handle, glm::vec3* color)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		material->GetSubMaterialAt(0)->GetMaterialData().emissiveColor = *color;
		Renderer::UpdateMaterial(material->GetSubMaterialAt(0).get());
	}

	inline static void Material_GetEmissiveColor(uint64_t handle, glm::vec3* outColor)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		*outColor = material->GetSubMaterialAt(0)->GetMaterialData().emissiveColor;
	}

	inline static void Material_SetEmissiveStrength(uint64_t handle, float strength)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		material->GetSubMaterialAt(0)->GetMaterialData().emissiveStrength = strength;
		Renderer::UpdateMaterial(material->GetSubMaterialAt(0).get());
	}

	inline static float Material_GetEmissiveStrength(uint64_t handle)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return 0.f;
		}
		return material->GetSubMaterialAt(0)->GetMaterialData().emissiveStrength;
	}

	inline static void Material_SetRoughness(uint64_t handle, float roughness)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		material->GetSubMaterialAt(0)->GetMaterialData().roughness = roughness;
		Renderer::UpdateMaterial(material->GetSubMaterialAt(0).get());
	}

	inline static float Material_GetRoughness(uint64_t handle)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return 0.f;
		}
		return material->GetSubMaterialAt(0)->GetMaterialData().roughness;
	}

	inline static void Material_SetMetalness(uint64_t handle, float metalness)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		material->GetSubMaterialAt(0)->GetMaterialData().metalness = metalness;
		Renderer::UpdateMaterial(material->GetSubMaterialAt(0).get());
	}

	inline static float Material_GetMetalness(uint64_t handle)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return 0.f;
		}
		return material->GetSubMaterialAt(0)->GetMaterialData().metalness;
	}

	inline static void Material_SetNormalStrength(uint64_t handle, float strength)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return;
		}
		material->GetSubMaterialAt(0)->GetMaterialData().normalStrength = strength;
		Renderer::UpdateMaterial(material->GetSubMaterialAt(0).get());
	}

	inline static float Material_GetNormalStrength(uint64_t handle)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set value on invalid material!");
			return 0.f;
		}
		return material->GetSubMaterialAt(0)->GetMaterialData().normalStrength;
	}

	inline static void Material_SetBool(uint64_t handle, MonoString* name, bool value)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->GetSubMaterialAt(0)->SetValue(param, value);
	}

	inline static void Material_SetInt(uint64_t handle, MonoString* name, int32_t value)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->GetSubMaterialAt(0)->SetValue(param, value);
	}

	inline static void Material_SetUInt(uint64_t handle, MonoString* name, uint32_t value)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->GetSubMaterialAt(0)->SetValue(param, value);
	}

	inline static void Material_SetFloat(uint64_t handle, MonoString* name, float value)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->GetSubMaterialAt(0)->SetValue(param, value);
	}

	inline static void Material_SetFloat2(uint64_t handle, MonoString* name, glm::vec2* value)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->GetSubMaterialAt(0)->SetValue(param, *value);
	}

	inline static void Material_SetFloat3(uint64_t handle, MonoString* name, glm::vec3* value)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->GetSubMaterialAt(0)->SetValue(param, *value);
	}

	inline static void Material_SetFloat4(uint64_t handle, MonoString* name, glm::vec4* value)
	{
		Ref<Material> material = AssetManager::GetAsset<Material>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->GetSubMaterialAt(0)->SetValue(param, *value);
	}

	inline static uint64_t Material_CreateCopy(uint64_t materialToCopy)
	{
		Ref<Material> originalMaterial = AssetManager::GetAsset<Material>(materialToCopy);
		if (!originalMaterial || !originalMaterial->IsValid())
		{
			VT_CORE_ERROR("Unable to copy invalid material!");
			return 0;
		}

		Ref<Material> copiedMaterial = AssetManager::CreateMemoryAsset<Material>(*originalMaterial);
		return copiedMaterial->handle;
	}
#pragma endregion

#pragma region PostProcessingMaterial
	inline static void PostProcessingMaterial_SetBool(uint64_t handle, MonoString* name, bool value)
	{
		Ref<PostProcessingMaterial> material = AssetManager::GetAsset<PostProcessingMaterial>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->SetValue(param, value);
	}

	inline static void PostProcessingMaterial_SetInt(uint64_t handle, MonoString* name, int32_t value)
	{
		Ref<PostProcessingMaterial> material = AssetManager::GetAsset<PostProcessingMaterial>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->SetValue(param, value);
	}

	inline static void PostProcessingMaterial_SetUInt(uint64_t handle, MonoString* name, uint32_t value)
	{
		Ref<PostProcessingMaterial> material = AssetManager::GetAsset<PostProcessingMaterial>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->SetValue(param, value);
	}

	inline static void PostProcessingMaterial_SetFloat(uint64_t handle, MonoString* name, float value)
	{
		Ref<PostProcessingMaterial> material = AssetManager::GetAsset<PostProcessingMaterial>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->SetValue(param, value);
	}

	inline static void PostProcessingMaterial_SetFloat2(uint64_t handle, MonoString* name, glm::vec2* value)
	{
		Ref<PostProcessingMaterial> material = AssetManager::GetAsset<PostProcessingMaterial>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->SetValue(param, *value);
	}

	inline static void PostProcessingMaterial_SetFloat3(uint64_t handle, MonoString* name, glm::vec3* value)
	{
		Ref<PostProcessingMaterial> material = AssetManager::GetAsset<PostProcessingMaterial>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->SetValue(param, *value);
	}

	inline static void PostProcessingMaterial_SetFloat4(uint64_t handle, MonoString* name, glm::vec4* value)
	{
		Ref<PostProcessingMaterial> material = AssetManager::GetAsset<PostProcessingMaterial>(handle);
		if (!material || !material->IsValid())
		{
			VT_CORE_ERROR("Trying to set parameter on invalid material!");
			return;
		}

		const std::string param = MonoScriptUtils::GetStringFromMonoString(name);
		material->SetValue(param, *value);
	}
#pragma endregion

#pragma region GraphKey

	inline static void GraphKey_DispatchEvent(entt::entity entityId, MonoString* eventName)
	{
		auto event = MonoScriptUtils::GetStringFromMonoString(eventName);
		auto scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };

		if (entity.HasComponent<Volt::VisualScriptingComponent>())
		{
			auto graph = entity.GetComponent<Volt::VisualScriptingComponent>().graph;
			if (!graph) return;
			for (auto eventOnEntity : graph->GetEvents())
			{
				if (eventOnEntity.name == event)
				{
					graph->GetEventSystem().Dispatch(eventOnEntity.id)
						;
					return;
				}
			}
		}
	}

#pragma endregion

#pragma region UIRenderer
	inline static void UIRenderer_SetViewport(float x, float y, float width, float height)
	{
		UIRenderer::SetViewport(x, y, width, height);
	}

	inline static void UIRenderer_SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
	{
		UIRenderer::SetScissor(x, y, width, height);
	}

	inline static void UIRenderer_DrawSprite(glm::vec3* position, glm::vec2* scale, float rotation, glm::vec4* color, glm::vec2* offset)
	{
		UIRenderer::DrawSprite(*position, *scale, rotation, *color, *offset);
	}

	inline static void UIRenderer_DrawSpriteTexture(uint64_t textureHandle, glm::vec3* position, glm::vec2* scale, float rotation, glm::vec4* color, glm::vec2* offset)
	{
		UIRenderer::DrawSprite(AssetManager::GetAsset<Texture2D>(textureHandle), *position, *scale, rotation, *color, *offset);
	}

	inline static void UIRenderer_DrawString(MonoString* text, uint64_t fontHandle, glm::vec3* position, glm::vec2* scale, float rotation, float maxWidth, glm::vec4* color, glm::vec2* positionOffset)
	{
		Ref<Font> font = AssetManager::GetAsset<Font>(fontHandle);
		if (!font || !font->IsValid())
		{
			return;
		}

		const std::string strText = MonoScriptUtils::GetStringFromMonoString(text);
		UIRenderer::DrawString(strText, font, *position, *scale, rotation, maxWidth, *color, *positionOffset);
	}
#pragma endregion UIRenderer

#pragma region Animation
	inline static float Animation_GetDuration(uint64_t handle)
	{
		Ref<Animation> animation = AssetManager::GetAsset<Animation>(handle);
		if (!animation || !animation->IsValid())
		{
			return 0.f;
		}

		return animation->GetDuration();
	}
#pragma endregion Animation

#pragma region Net
	inline static Nexus::TYPE::REP_ID NetActorComponent_GetRepId(uint32_t id)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		auto entity = Entity(id, scene);
		while (!entity.HasComponent<NetActorComponent>())
		{
			if (entity.GetParent().GetID() == 0) return 0;
			entity = entity.GetParent();
		}
		auto netId = entity.GetComponent<NetActorComponent>().repId;
		return (Nexus::TYPE::REP_ID)netId;
	}

	inline static void NetEvent_TriggerEventFromNetId(Nexus::TYPE::REP_ID id, eNetEvent netEvent, MonoArray* data)
	{
		std::vector<uint8_t> byteVec;
		byteVec.resize(mono_array_length(data));

		for (int i = 0; i < byteVec.size(); i++)
		{
			byteVec[i] = mono_array_get(data, uint8_t, i);
		}

		auto& handler = Application::Get().GetNetHandler();
		if (handler.IsHost())
		{
			Application::Get().GetNetHandler().GetEventContainer().AddIncomming(NetEvent(netEvent, id, byteVec));
			return;
		}
		Application::Get().GetNetHandler().GetEventContainer().AddOutgoing(NetEvent(netEvent, id, byteVec));
	}

	inline static void NetEvent_TriggerEventFromLocalId(uint32_t id, eNetEvent netEvent, MonoArray* data)
	{
		NetEvent_TriggerEventFromNetId(NetActorComponent_GetRepId(id), netEvent, data);
	}

	inline static void NetScene_DestroyFromNetId(Nexus::TYPE::REP_ID repId)
	{
		auto& handler = Application::Get().GetNetHandler();
		auto& backend = handler.GetBackend();

		if (!backend)
		{
			return;
		}

		Nexus::Packet packet;
		packet.ownerID = handler.GetBackend()->GetClientId();
		packet.id = Nexus::ePacketID::REMOVE_ENTITY;
		packet << repId;

		if (handler.IsHost())
		{
			// #nexus_todo: add internal queue support
			backend->AddPacketToIncomming(packet);
			return;
		}
		else backend->Transmit(packet);
	}

	inline static void NetScene_DestroyFromLocalId(uint32_t id)
	{
		NetScene_DestroyFromNetId(NetActorComponent_GetRepId(id));
	}

	inline static void NetScene_InstantiatePrefabAtEntity(uint64_t handle, uint32_t spawnPoint)
	{
		auto& handler = Application::Get().GetNetHandler();

		auto& backend = handler.GetBackend();
		if (!backend) { return; }
		auto clientId = handler.GetBackend()->GetClientId();
		auto prefabData = CreatePrefabData(0, clientId, handle);

		auto scene = MonoScriptEngine::GetSceneContext();
		auto ent = Entity(spawnPoint, scene);

		TransformComponent temp;
		temp.position = ent.GetPosition();
		temp.rotation = ent.GetRotation();
		temp.scale = ent.GetScale();

		auto transformData = CreateTransformComponentData(0, temp);

		Nexus::Packet packet;
		packet.ownerID = clientId;
		packet.id = Nexus::ePacketID::CREATE_ENTITY;
		packet < transformData < prefabData;

		if (handler.IsHost())
		{
			// #nexus_todo: add internal queue support
			backend->AddPacketToIncomming(packet);
			return;
		}
		backend->Transmit(packet);
	}

	inline static void Net_NotifyFromNetId(Nexus::TYPE::REP_ID repId, MonoString* fieldName)
	{
		if (!fieldName) return;
		const std::string str = MonoScriptUtils::GetStringFromMonoString(fieldName);
		if (str.empty()) return;

		auto& handler = Application::Get().GetNetHandler();
		auto& backend = handler.GetBackend();
		if (!backend) { return; }

		auto varId = backend->GetRegistry().GetLinked(repId, str);
		auto repVar = backend->GetRegistry().GetAs<RepVariable>(varId);
		if (!repVar) return;

		Nexus::Packet packet = SerializeVariablePacket(*repVar, varId);
		if (handler.IsHost())
		{
			// #nexus_todo: add internal queue support
			backend->AddPacketToIncomming(packet);
			return;
		}
		backend->AddPacketToIncomming(packet);
		backend->Transmit(packet);
	}

	inline static void Net_Notify(uint32_t repId, MonoString* fieldName)
	{
		Net_NotifyFromNetId(NetActorComponent_GetRepId(repId), fieldName);
	}

	inline static void Net_StartClient()
	{
		Application::Get().GetNetHandler().StartClient();
	}

	inline static void Net_StartServer()
	{
		Application::Get().GetNetHandler().StartServer();
	}

	inline static void Net_StartSinglePlayer()
	{
		Application::Get().GetNetHandler().StartSinglePlayer();
	}

	inline static void Net_SetHandleTick(bool value)
	{
		Application::Get().GetNetHandler().EnableTick(value);
	}

	inline static void Net_Connect(MonoString* ip, unsigned short port)
	{
		auto& netHandler = Application::Get().GetNetHandler();
		if (!netHandler.GetBackend())
		{
			// 
			return;
		}
		if (netHandler.IsHost())
		{
			Nexus::Packet connectionPacket;
			connectionPacket.id = Nexus::ePacketID::CONNECT;
			connectionPacket << std::string("Host Client");
			netHandler.GetBackend()->GetIncommingPacketQueue().push_back({ Nexus::CreateSockAddr("127.0.0.1", netHandler.GetBackend()->GetRelay().GetBoundPort()),connectionPacket });
			return;
		}

		if (!ip) return;
		const std::string formattedIp = MonoScriptUtils::GetStringFromMonoString(ip);
		if (formattedIp.empty()) return;

		//netHandler.StartClient();
		// #nexus_todo: failsafe ip
		((Volt::NetClient*)(netHandler.GetBackend().get()))->StoreServerData(formattedIp, port);
		Nexus::Packet connectionPacket;
		connectionPacket.id = Nexus::ePacketID::CONNECT;
		connectionPacket << std::string("Connected Client");
		netHandler.GetBackend()->Transmit(connectionPacket);
	}

	inline static void Net_Disconnect()
	{
		auto& netHandler = Application::Get().GetNetHandler();
		if (!netHandler.GetBackend()) return;

		Nexus::Packet packet;
		packet.id = Nexus::ePacketID::DISCONNECT;
		packet.ownerID = netHandler.GetBackend()->GetClientId();

		if (netHandler.IsHost())
		{
			netHandler.GetBackend()->GetIncommingPacketQueue().push_back({ Nexus::CreateSockAddr("127.0.0.1", netHandler.GetBackend()->GetRelay().GetBoundPort()),packet });
			return;
		}
		netHandler.GetBackend()->Transmit(packet);
	}

	inline static void Net_SceneLoad()
	{
		Volt::Application::Get().GetNetHandler().LoadNetScene();
	}

	inline static void Net_InstantiatePlayer()
	{
		auto& netHandler = Application::Get().GetNetHandler();
		if (!netHandler.GetBackend()) return;

		if (netHandler.IsHost())
		{
			Nexus::Packet connectionPacket;
			connectionPacket.id = Nexus::ePacketID::CONSTRUCT_REGISTRY;
			netHandler.GetBackend()->GetIncommingPacketQueue().push_back({ Nexus::CreateSockAddr("127.0.0.1", netHandler.GetBackend()->GetRelay().GetBoundPort()),connectionPacket });
			return;
		}

		Nexus::Packet connectionPacket;
		connectionPacket.id = Nexus::ePacketID::CONSTRUCT_REGISTRY;
		netHandler.GetBackend()->Transmit(connectionPacket);
	}

	inline static void Net_Reload()
	{
		Application::Get().GetNetHandler().Reload();
	}

	inline static void Net_ForcePortBinding(unsigned short port)
	{
		Application::Get().GetNetHandler().SetForcedPort(port);
	}

	inline static bool Net_IsHost()
	{
		return Application::Get().GetNetHandler().IsHost();
	}

	inline static unsigned short Net_GetBoundPort()
	{
		auto& backend = Volt::Application::Get().GetNetHandler().GetBackend();
		if (!backend) return 0;
		return backend->GetRelay().GetBoundPort();
	}

	void MonoScriptGlue::Net_OnConnectCallback()
	{
		auto klass = MonoScriptClass(MonoScriptEngine::GetCoreAssembly().assemblyImage, "Volt", "Net");

		auto method = klass.GetMethod("OnConnectCallback", 0);
		if (!method)
		{
			return;
		}

		MonoScriptEngine::CallStaticMethod(method, nullptr);
	}
#pragma endregion Net

#pragma region Steam

	inline static void SteamAPI_StartLobby(MonoString* address)
	{
		Application::Get().GetSteam().StartLobby(MonoScriptUtils::GetStringFromMonoString(address));
	}

	void MonoScriptGlue::SteamAPI_Clean()
	{
		Application::Get().GetSteam().ClearRichPresence();
		auto klass = MonoScriptClass(MonoScriptEngine::GetCoreAssembly().assemblyImage, "Volt", "SteamAPI");
		MonoScriptEngine::CallStaticMethod(klass.GetMethod("Clean", 0), nullptr);
	}

	void MonoScriptGlue::SteamAPI_OnJoinRequest(std::string address)
	{
		auto klass = MonoScriptClass(MonoScriptEngine::GetCoreAssembly().assemblyImage, "Volt", "SteamAPI");
		void* args[1] = { MonoScriptUtils::GetMonoStringFromString(address) };
		MonoScriptEngine::CallStaticMethod(klass.GetMethod("OnJoinRequest", 1), args);
	}

	inline static void SteamAPI_SetStatInt(MonoString* name, int32_t value)
	{
		auto& steamAPI = Application::Get().GetSteam();
		const std::string str = MonoScriptUtils::GetStringFromMonoString(name);
		steamAPI.SetStat(str, value);
	}

	inline static void SteamAPI_SetStatFloat(MonoString* name, float value)
	{
		auto& steamAPI = Application::Get().GetSteam();
		const std::string str = MonoScriptUtils::GetStringFromMonoString(name);
		steamAPI.SetStat(str, value);
	}

	inline static int32_t SteamAPI_GetStatInt(MonoString* name)
	{
		auto& steamAPI = Application::Get().GetSteam();
		const std::string str = MonoScriptUtils::GetStringFromMonoString(name);

		return steamAPI.GetStatInt(str);
	}

	inline static float SteamAPI_GetStatFloat(MonoString* name)
	{
		auto& steamAPI = Application::Get().GetSteam();
		const std::string str = MonoScriptUtils::GetStringFromMonoString(name);

		return steamAPI.GetStatFloat(str);
	}

	inline static bool SteamAPI_IsStatsValid()
	{
		auto& steamAPI = Application::Get().GetSteam();
		return steamAPI.IsStatsValid();
	}

	inline static void SteamAPI_StoreStats()
	{
		auto& steamAPI = Application::Get().GetSteam();
		steamAPI.StoreStats();
	}

	inline static void SteamAPI_RequestStats()
	{
		auto& steamAPI = Application::Get().GetSteam();
		steamAPI.RequestStats();
	}

#pragma endregion Steam

#pragma region Window
	inline static float Window_GetWidth()
	{
		if (Application::Get().IsRuntime())
		{
			return static_cast<float>(Application::Get().GetWindow().GetWidth());
		}

		return static_cast<float>(Application::Get().GetWindow().GetViewportWidth());
	}

	inline static float Window_GetHeight()
	{
		if (Application::Get().IsRuntime())
		{
			return static_cast<float>(Application::Get().GetWindow().GetHeight());
		}

		return static_cast<float>(Application::Get().GetWindow().GetViewportHeight());
	}

	inline static void Window_SetCursor(MonoString* path)
	{
		const std::string str = MonoScriptUtils::GetStringFromMonoString(path);
		const std::filesystem::path contextPath = AssetManager::GetContextPath(str);
		Application::Get().GetWindow().SetCursor(str);
	}

#pragma endregion Window

#pragma region Asset
	inline static bool Asset_IsValid(uint64_t handle)
	{
		Ref<Asset> assetRaw = AssetManager::Get().GetAssetRaw(handle);
		if (assetRaw && assetRaw->IsValid())
		{
			return true;
		}

		return false;
	}
#pragma endregion Asset

#pragma region Font
	inline static float Font_GetStringWidth(uint64_t fontHandle, MonoString* text, glm::vec2* scale, float maxWidth)
	{
		if (!text)
		{
			return 0.f;
		}

		const std::string str = MonoScriptUtils::GetStringFromMonoString(text);
		if (str.empty())
		{
			return 0.f;
		}

		Ref<Font> font = AssetManager::GetAsset<Font>(fontHandle);
		if (!font || !font->IsValid())
		{
			return 0.f;
		}

		const float currentScaleModifier = UIRenderer::GetCurrentScaleModifier();
		const float textWidth = font->GetStringWidth(str, (*scale) * currentScaleModifier, maxWidth);

		return textWidth;
	}

	inline static float Font_GetStringHeight(uint64_t fontHandle, MonoString* text, glm::vec2* scale, float maxWidth)
	{
		if (!text)
		{
			return 0.f;
		}

		const std::string str = MonoScriptUtils::GetStringFromMonoString(text);
		if (str.empty())
		{
			return 0.f;
		}

		Ref<Font> font = AssetManager::GetAsset<Font>(fontHandle);
		if (!font || !font->IsValid())
		{
			return 0.f;
		}

		const float currentScaleModifier = UIRenderer::GetCurrentScaleModifier();
		const float textHeight = font->GetStringHeight(str, (*scale) * currentScaleModifier, maxWidth);

		return textHeight;
	}
#pragma endregion

#pragma region PostProcessingStack
	inline static void PostProcessingStack_PushEffect(uint64_t effectHandle)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto& postStackComponents = scene->GetRegistry().GetSingleComponentView<PostProcessingStackComponent>();
		if (postStackComponents.empty())
		{
			return;
		}

		const auto& postStackComp = scene->GetRegistry().GetComponent<PostProcessingStackComponent>(postStackComponents.front());

		Ref<PostProcessingStack> postStack = AssetManager::GetAsset<PostProcessingStack>(postStackComp.postProcessingStack);
		if (!postStack || !postStack->IsValid())
		{
			return;
		}

		Ref<PostProcessingMaterial> postMat = AssetManager::GetAsset<PostProcessingMaterial>(postStackComp.postProcessingStack);
		if (!postMat || !postMat->IsValid())
		{
			return;
		}

		postStack->PushEffect({ postMat->handle });
	}

	inline static void PostProcessingStack_PopEffect()
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		const auto& postStackComponents = scene->GetRegistry().GetSingleComponentView<PostProcessingStackComponent>();
		if (postStackComponents.empty())
		{
			return;
		}

		const auto& postStackComp = scene->GetRegistry().GetComponent<PostProcessingStackComponent>(postStackComponents.front());

		Ref<PostProcessingStack> postStack = AssetManager::GetAsset<PostProcessingStack>(postStackComp.postProcessingStack);
		if (!postStack || !postStack->IsValid())
		{
			return;
		}

		postStack->PopEffect();
	}
#pragma endregion

#pragma region Renderer
	inline static void Renderer_SetRenderScale(float renderScale)
	{
		OnRenderScaleChangedEvent renderScaleEvent{ renderScale };
		Application::Get().OnEvent(renderScaleEvent);
	}

	inline static void Renderer_SetRendererSettings(SceneRendererSettings* rendererSettings)
	{
		OnRendererSettingsChangedEvent settingsEvent{ *rendererSettings };
		Application::Get().OnEvent(settingsEvent);
	}
#pragma endregion Renderer

#pragma region VideoPlayer
	inline static void VideoPlayer_Play(uint64_t assetHandle, bool loop)
	{
		Ref<Video> video = AssetManager::GetAsset<Video>(assetHandle);
		if (!video || !video->IsValid())
		{
			return;
		}

		video->Play(loop);
	}

	inline static void VideoPlayer_Stop(uint64_t assetHandle)
	{
		Ref<Video> video = AssetManager::GetAsset<Video>(assetHandle);
		if (!video || !video->IsValid())
		{
			return;
		}

		video->Stop();
	}

	inline static void VideoPlayer_Restart(uint64_t assetHandle)
	{
		Ref<Video> video = AssetManager::GetAsset<Video>(assetHandle);
		if (!video || !video->IsValid())
		{
			return;
		}

		video->Restart();
	}

	inline static void VideoPlayer_Pause(uint64_t assetHandle)
	{
		Ref<Video> video = AssetManager::GetAsset<Video>(assetHandle);
		if (!video || !video->IsValid())
		{
			return;
		}

		video->Pause();
	}

	inline static void VideoPlayer_Update(uint64_t assetHandle, float deltaTime)
	{
		Ref<Video> video = AssetManager::GetAsset<Video>(assetHandle);
		if (!video || !video->IsValid())
		{
			return;
		}

		video->Update(deltaTime);
	}

	inline static void VideoPlayer_DrawSpriteTexture(uint64_t videoHandle, glm::vec3* position, glm::vec2* scale, float rotation, glm::vec4* color, glm::vec2* offset)
	{
		Ref<Video> video = AssetManager::GetAsset<Video>(videoHandle);
		if (!video || !video->IsValid())
		{
			return;
		}

		UIRenderer::DrawSprite(video->GetImage(), *position, *scale, rotation, *color, *offset);
	}

	inline static VideoStatus VideoPlayer_GetStatus(uint64_t videoHandle)
	{
		Ref<Video> video = AssetManager::GetAsset<Video>(videoHandle);
		if (!video || !video->IsValid())
		{
			return VideoStatus::Stopped;
		}

		return video->GetStatus();
	}
#pragma endregion

	void MonoScriptGlue::RegisterFunctions()
	{
		//VoltApplication
		{
			VT_ADD_INTERNAL_CALL(VoltApplication_LoadLevel);
			VT_ADD_INTERNAL_CALL(VoltApplication_IsRuntime);
			VT_ADD_INTERNAL_CALL(VoltApplication_Quit);
			VT_ADD_INTERNAL_CALL(VoltApplication_GetClipboard);
			VT_ADD_INTERNAL_CALL(VoltApplication_SetClipboard);

			VT_ADD_INTERNAL_CALL(VoltApplication_SetResolution);
			VT_ADD_INTERNAL_CALL(VoltApplication_SetWindowMode);
		}

		// Net
		{
			VT_ADD_INTERNAL_CALL(NetActorComponent_GetRepId);
			VT_ADD_INTERNAL_CALL(NetEvent_TriggerEventFromLocalId);
			VT_ADD_INTERNAL_CALL(NetEvent_TriggerEventFromNetId);

			VT_ADD_INTERNAL_CALL(NetScene_InstantiatePrefabAtEntity);
			VT_ADD_INTERNAL_CALL(NetScene_DestroyFromNetId);
			VT_ADD_INTERNAL_CALL(NetScene_DestroyFromLocalId);

			VT_ADD_INTERNAL_CALL(Net_SceneLoad);
			VT_ADD_INTERNAL_CALL(Net_Notify);
			VT_ADD_INTERNAL_CALL(Net_NotifyFromNetId);

			VT_ADD_INTERNAL_CALL(Net_StartServer);
			VT_ADD_INTERNAL_CALL(Net_StartClient);
			VT_ADD_INTERNAL_CALL(Net_StartSinglePlayer);
			VT_ADD_INTERNAL_CALL(Net_SetHandleTick);
			VT_ADD_INTERNAL_CALL(Net_Disconnect);
			VT_ADD_INTERNAL_CALL(Net_Connect);
			VT_ADD_INTERNAL_CALL(Net_InstantiatePlayer);
			VT_ADD_INTERNAL_CALL(Net_GetBoundPort);
			VT_ADD_INTERNAL_CALL(Net_ForcePortBinding);
			VT_ADD_INTERNAL_CALL(Net_Reload);
			VT_ADD_INTERNAL_CALL(Net_IsHost);
		}

		// Graph Key
		{
			VT_ADD_INTERNAL_CALL(GraphKey_DispatchEvent);
		}

		// Entity
		{
			VT_ADD_INTERNAL_CALL(Entity_IsValid);
			VT_ADD_INTERNAL_CALL(Entity_HasComponent);
			VT_ADD_INTERNAL_CALL(Entity_HasScript);
			VT_ADD_INTERNAL_CALL(Entity_GetScript);
			VT_ADD_INTERNAL_CALL(Entity_RemoveComponent);
			VT_ADD_INTERNAL_CALL(Entity_RemoveScript);
			VT_ADD_INTERNAL_CALL(Entity_AddComponent);
			VT_ADD_INTERNAL_CALL(Entity_AddScript);
			VT_ADD_INTERNAL_CALL(Entity_FindByName);
			VT_ADD_INTERNAL_CALL(Entity_FindById);
			VT_ADD_INTERNAL_CALL(Entity_CreateNewEntity);
			VT_ADD_INTERNAL_CALL(Entity_CreateNewEntityWithPrefab);
			VT_ADD_INTERNAL_CALL(Entity_DeleteEntity);
			VT_ADD_INTERNAL_CALL(Entity_Clone);
		}

		// Asset
		{
			VT_ADD_INTERNAL_CALL(AssetManager_HasAsset);
			VT_ADD_INTERNAL_CALL(AssetManager_GetAssetHandleFromPath);
		}

		// Scene
		{
			VT_ADD_INTERNAL_CALL(Scene_Load);
			VT_ADD_INTERNAL_CALL(Scene_Save);
			VT_ADD_INTERNAL_CALL(Scene_Preload);
			VT_ADD_INTERNAL_CALL(Scene_GetAllEntities);
			VT_ADD_INTERNAL_CALL(Scene_GetAllEntitiesWithComponent);
			VT_ADD_INTERNAL_CALL(Scene_GetAllEntitiesWithScript);
			VT_ADD_INTERNAL_CALL(Scene_InstantiateSplitMesh);
			VT_ADD_INTERNAL_CALL(Scene_CreateDynamicPhysicsActor);
		}

		// Transform component
		{
			VT_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
			VT_ADD_INTERNAL_CALL(TransformComponent_SetPosition);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
			VT_ADD_INTERNAL_CALL(TransformComponent_SetRotation);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetScale);
			VT_ADD_INTERNAL_CALL(TransformComponent_SetScale);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetForward);
			VT_ADD_INTERNAL_CALL(TransformComponent_GetRight);
			VT_ADD_INTERNAL_CALL(TransformComponent_GetUp);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetLocalPosition);
			VT_ADD_INTERNAL_CALL(TransformComponent_SetLocalPosition);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetLocalRotation);
			VT_ADD_INTERNAL_CALL(TransformComponent_SetLocalRotation);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetLocalScale);
			VT_ADD_INTERNAL_CALL(TransformComponent_SetLocalScale);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetLocalForward);
			VT_ADD_INTERNAL_CALL(TransformComponent_GetLocalRight);
			VT_ADD_INTERNAL_CALL(TransformComponent_GetLocalUp);

			VT_ADD_INTERNAL_CALL(TransformComponent_GetVisible);
			VT_ADD_INTERNAL_CALL(TransformComponent_SetVisible);
		}

		// Tag Component
		{
			VT_ADD_INTERNAL_CALL(TagComponent_SetTag);
			VT_ADD_INTERNAL_CALL(TagComponent_GetTag);
		}

		// PointLightComponent
		{
			VT_ADD_INTERNAL_CALL(PointlightComponent_GetColor);
			VT_ADD_INTERNAL_CALL(PointlightComponent_SetColor);
			VT_ADD_INTERNAL_CALL(PointlightComponent_GetIntensity);
			VT_ADD_INTERNAL_CALL(PointlightComponent_SetIntensity);
		}

		// PointLightComponent
		{
			VT_ADD_INTERNAL_CALL(SpotlightComponent_GetColor);
			VT_ADD_INTERNAL_CALL(SpotlightComponent_SetColor);
			VT_ADD_INTERNAL_CALL(SpotlightComponent_GetIntensity);
			VT_ADD_INTERNAL_CALL(SpotlightComponent_SetIntensity);
		}

		// Relationship Component
		{
			VT_ADD_INTERNAL_CALL(RelationshipComponent_SetParent);
			VT_ADD_INTERNAL_CALL(RelationshipComponent_FindByName);
			VT_ADD_INTERNAL_CALL(RelationshipComponent_GetParent);
			VT_ADD_INTERNAL_CALL(RelationshipComponent_GetChildren);
		}

		// Rigidbody Component
		{
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetCollisionDetectionType);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetCollisionDetectionType);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetDisableGravity);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetDisableGravity);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetIsKinematic);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetIsKinematic);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetLockFlags);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetLockFlags);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetBodyType);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetBodyType);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetLayerId);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetLayerId);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetMass);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetMass);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetLinearDrag);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetLinearDrag);

			VT_ADD_INTERNAL_CALL(RigidbodyComponent_GetAngularDrag);
			VT_ADD_INTERNAL_CALL(RigidbodyComponent_SetAngularDrag);
		}

		// Character Controller Component
		{
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetSlopeLimit);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetSlopeLimit);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetInvisibleWallHeight);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetInvisibleWallHeight);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetMaxJumpHeight);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetMaxJumpHeight);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetContactOffset);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetContactOffset);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetStepOffset);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetStepOffset);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetDensity);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetDensity);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetGravity);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetGravity);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetAngularVelocity);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetAngularVelocity);

			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_GetLinearVelocity);
			VT_ADD_INTERNAL_CALL(CharacterControllerComponent_SetLinearVelocity);
		}

		// Box Collider Component
		{
			VT_ADD_INTERNAL_CALL(BoxColliderComponent_GetHalfSize);
			VT_ADD_INTERNAL_CALL(BoxColliderComponent_SetHalfSize);

			VT_ADD_INTERNAL_CALL(BoxColliderComponent_GetOffset);
			VT_ADD_INTERNAL_CALL(BoxColliderComponent_SetOffset);

			VT_ADD_INTERNAL_CALL(BoxColliderComponent_GetIsTrigger);
			VT_ADD_INTERNAL_CALL(BoxColliderComponent_SetIsTrigger);
		}

		// Sphere Collider Component
		{
			VT_ADD_INTERNAL_CALL(SphereColliderComponent_GetRadius);
			VT_ADD_INTERNAL_CALL(SphereColliderComponent_SetRadius);

			VT_ADD_INTERNAL_CALL(SphereColliderComponent_GetOffset);
			VT_ADD_INTERNAL_CALL(SphereColliderComponent_SetOffset);

			VT_ADD_INTERNAL_CALL(SphereColliderComponent_GetIsTrigger);
			VT_ADD_INTERNAL_CALL(SphereColliderComponent_SetIsTrigger);
		}

		// Capsule Collider Component
		{
			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetRadius);
			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetRadius);

			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetHeight);
			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetHeight);

			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetOffset);
			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetOffset);

			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_GetIsTrigger);
			VT_ADD_INTERNAL_CALL(CapsuleColliderComponent_SetIsTrigger);
		}

		// Mesh Collider Component
		{
			VT_ADD_INTERNAL_CALL(MeshColliderComponent_SetIsConvex);
			VT_ADD_INTERNAL_CALL(MeshColliderComponent_GetIsConvex);

			VT_ADD_INTERNAL_CALL(MeshColliderComponent_GetSubMeshIndex);
			VT_ADD_INTERNAL_CALL(MeshColliderComponent_SetSubMeshIndex);

			VT_ADD_INTERNAL_CALL(MeshColliderComponent_GetIsTrigger);
			VT_ADD_INTERNAL_CALL(MeshColliderComponent_SetIsTrigger);

			VT_ADD_INTERNAL_CALL(MeshColliderComponent_SetColliderMesh);
			VT_ADD_INTERNAL_CALL(MeshColliderComponent_GetColliderMesh);
		}

		// Physics Actor
		{
			VT_ADD_INTERNAL_CALL(PhysicsActor_SetKinematicTarget);

			VT_ADD_INTERNAL_CALL(PhysicsActor_SetLinearVelocity);
			VT_ADD_INTERNAL_CALL(PhysicsActor_SetMaxLinearVelocity);

			VT_ADD_INTERNAL_CALL(PhysicsActor_SetAngularVelocity);
			VT_ADD_INTERNAL_CALL(PhysicsActor_SetMaxAngularVelocity);

			VT_ADD_INTERNAL_CALL(PhysicsActor_GetAngularVelocity);
			VT_ADD_INTERNAL_CALL(PhysicsActor_GetLinearVelocity);

			VT_ADD_INTERNAL_CALL(PhysicsActor_GetKinematicTargetPosition);
			VT_ADD_INTERNAL_CALL(PhysicsActor_GetKinematicTargetRotation);

			VT_ADD_INTERNAL_CALL(PhysicsActor_AddForce);
			VT_ADD_INTERNAL_CALL(PhysicsActor_AddTorque);

			VT_ADD_INTERNAL_CALL(PhysicsActor_WakeUp);
			VT_ADD_INTERNAL_CALL(PhysicsActor_PutToSleep);
		}

		// Physics Actor Controller
		{
			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_GetHeight);
			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_SetHeight);

			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_GetRadius);
			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_SetRadius);

			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_Move);
			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_SetPosition);

			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_SetFootPosition);
			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_GetPosition);

			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_GetFootPosition);
			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_IsGrounded);

			VT_ADD_INTERNAL_CALL(PhysicsControllerActor_Jump);
		}

		// Physics
		{
			VT_ADD_INTERNAL_CALL(Physics_GetGravity);
			VT_ADD_INTERNAL_CALL(Physics_SetGravity);
			VT_ADD_INTERNAL_CALL(Physics_Raycast);
			VT_ADD_INTERNAL_CALL(Physics_RaycastLayerMask);
			VT_ADD_INTERNAL_CALL(Physics_OverlapBox);
			VT_ADD_INTERNAL_CALL(Physics_OverlapSphere);
		}

		// InputMapper
		{
			VT_ADD_INTERNAL_CALL(InputMapper_GetKey);
			VT_ADD_INTERNAL_CALL(InputMapper_SetKey);
			VT_ADD_INTERNAL_CALL(InputMapper_ResetKey);
		}

		// Input
		{
			VT_ADD_INTERNAL_CALL(Input_KeyPressed);
			VT_ADD_INTERNAL_CALL(Input_GetAllKeyPressed);
			VT_ADD_INTERNAL_CALL(Input_KeyReleased);
			VT_ADD_INTERNAL_CALL(Input_MousePressed);
			VT_ADD_INTERNAL_CALL(Input_MouseReleased);
			VT_ADD_INTERNAL_CALL(Input_KeyDown);
			VT_ADD_INTERNAL_CALL(Input_KeyUp);
			VT_ADD_INTERNAL_CALL(Input_MouseDown);
			VT_ADD_INTERNAL_CALL(Input_MouseUp);
			VT_ADD_INTERNAL_CALL(Input_SetMousePosition);
			VT_ADD_INTERNAL_CALL(Input_GetMousePosition);
			VT_ADD_INTERNAL_CALL(Input_ShowCursor);
			VT_ADD_INTERNAL_CALL(Input_GetScrollOffset);
		}

		// Math
		{
			VT_ADD_INTERNAL_CALL(Noise_Perlin);
			VT_ADD_INTERNAL_CALL(Noise_SetFrequency);
			VT_ADD_INTERNAL_CALL(Noise_SetSeed);
		}

		// Log
		{
			VT_ADD_INTERNAL_CALL(Log_String);
		}

		// AMP
		{
			VT_ADD_INTERNAL_CALL(AMP_PlayOneshotEvent);
			VT_ADD_INTERNAL_CALL(AMP_SetRTPC);
			VT_ADD_INTERNAL_CALL(AMP_StopAllEvents);

			VT_ADD_INTERNAL_CALL(AudioSourceComponent_PlayEvent);
			VT_ADD_INTERNAL_CALL(AudioSourceComponent_PlayOneshotEvent);

			VT_ADD_INTERNAL_CALL(AudioSourceComponent_StopEvent);
			VT_ADD_INTERNAL_CALL(AudioSourceComponent_StopAllEvents);
			VT_ADD_INTERNAL_CALL(AudioSourceComponent_PauseEvent);
			VT_ADD_INTERNAL_CALL(AudioSourceComponent_SetState);
			VT_ADD_INTERNAL_CALL(AudioSourceComponent_SetSwitch);
			VT_ADD_INTERNAL_CALL(AudioSourceComponent_SetParameter);
			VT_ADD_INTERNAL_CALL(AudioSourceComponent_SetParameterOverTime);



		}

		// Vision
		{
			VT_ADD_INTERNAL_CALL(Vision_DoCameraShake);
			VT_ADD_INTERNAL_CALL(Vision_SetActiveCamera);
			VT_ADD_INTERNAL_CALL(Vision_GetActiveCamera);
			VT_ADD_INTERNAL_CALL(Vision_SetCameraFollow);
			VT_ADD_INTERNAL_CALL(Vision_SetCameraLookAt);
			VT_ADD_INTERNAL_CALL(Vision_SetCameraFocusPoint);
			VT_ADD_INTERNAL_CALL(Vision_SetCameraFieldOfView);
			VT_ADD_INTERNAL_CALL(Vision_SetCameraDampAmount);
			VT_ADD_INTERNAL_CALL(Vision_SetCameraLocked);
			VT_ADD_INTERNAL_CALL(Vision_SetCameraMouseSensentivity);
		}

		// Render
		{
			VT_ADD_INTERNAL_CALL(DebugRenderer_DrawLine);
			VT_ADD_INTERNAL_CALL(DebugRenderer_DrawSprite);
			VT_ADD_INTERNAL_CALL(DebugRenderer_DrawText);
		}

		// Navigation
		{
			VT_ADD_INTERNAL_CALL(Navigation_FindPath);
		}

		// Nav Agent Component
		{
			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetTarget);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetTarget);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetPosition);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetPosition);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetVelocity);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetActive);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetActive);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetRadius);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetRadius);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetHeight);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetHeight);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetMaxSpeed);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetMaxSpeed);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetAcceleration);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetAcceleration);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetSeperationWeight);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetSeperationWeight);

			VT_ADD_INTERNAL_CALL(NavAgentComponent_GetObstacleAvoidanceQuality);
			VT_ADD_INTERNAL_CALL(NavAgentComponent_SetObstacleAvoidanceQuality);
		}

		// Visual Scripting Component
		{
			VT_ADD_INTERNAL_CALL(VisualScriptingComponent_SetParameterBool);
			VT_ADD_INTERNAL_CALL(VisualScriptingComponent_SetParameterInt);
			VT_ADD_INTERNAL_CALL(VisualScriptingComponent_SetParameterFloat);
			VT_ADD_INTERNAL_CALL(VisualScriptingComponent_SetParameterVector3);
			VT_ADD_INTERNAL_CALL(VisualScriptingComponent_SetParameterString);
		}

		// Animation Controller Component
		{
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetParameterBool);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetParameterInt);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetParameterFloat);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetParameterVector3);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetParameterString);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetBoundingSphere);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetParameterBool);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetParameterInt);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetParameterFloat);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetParameterVector3);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetParameterString);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetRootMotion);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_AttachEntity);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_DetachEntity);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_HasOverrideMaterial);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetOverrideMaterial);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetOverrideMaterial);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_GetOverrideSkin);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetOverrideSkin);
			VT_ADD_INTERNAL_CALL(AnimationControllerComponent_SetController);
		}

		// Text Renderer Component
		{
			VT_ADD_INTERNAL_CALL(TextRendererComponent_GetText);
			VT_ADD_INTERNAL_CALL(TextRendererComponent_SetText);
			VT_ADD_INTERNAL_CALL(TextRendererComponent_GetMaxWidth);
			VT_ADD_INTERNAL_CALL(TextRendererComponent_SetMaxWidth);
			VT_ADD_INTERNAL_CALL(TextRendererComponent_GetColor);
			VT_ADD_INTERNAL_CALL(TextRendererComponent_SetColor);
		}

		// Time
		{
			VT_ADD_INTERNAL_CALL(Time_GetDeltaTime);
			VT_ADD_INTERNAL_CALL(Time_SetTimeScale);
		}

		// Project
		{
			VT_ADD_INTERNAL_CALL(Project_GetDirectory);
			VT_ADD_INTERNAL_CALL(Project_GetCompanyName);
			VT_ADD_INTERNAL_CALL(Project_GetProjectName);
		}

		// Mesh Component
		{
			VT_ADD_INTERNAL_CALL(MeshComponent_GetMeshHandle);
			VT_ADD_INTERNAL_CALL(MeshComponent_SetMeshHandle);
			VT_ADD_INTERNAL_CALL(MeshComponent_HasOverrideMaterial);
			VT_ADD_INTERNAL_CALL(MeshComponent_GetOverrideMaterial);
			VT_ADD_INTERNAL_CALL(MeshComponent_SetOverrideMaterial);
		}

		// Mesh
		{
			VT_ADD_INTERNAL_CALL(Mesh_GetMaterial);
		}

		// Material
		{
			VT_ADD_INTERNAL_CALL(Material_SetBool);
			VT_ADD_INTERNAL_CALL(Material_SetInt);
			VT_ADD_INTERNAL_CALL(Material_SetUInt);
			VT_ADD_INTERNAL_CALL(Material_SetFloat);
			VT_ADD_INTERNAL_CALL(Material_SetFloat2);
			VT_ADD_INTERNAL_CALL(Material_SetFloat3);
			VT_ADD_INTERNAL_CALL(Material_SetFloat4);

			VT_ADD_INTERNAL_CALL(Material_CreateCopy);

			VT_ADD_INTERNAL_CALL(Material_SetColor);
			VT_ADD_INTERNAL_CALL(Material_GetColor);
			VT_ADD_INTERNAL_CALL(Material_SetEmissiveColor);
			VT_ADD_INTERNAL_CALL(Material_GetEmissiveColor);
			VT_ADD_INTERNAL_CALL(Material_SetEmissiveStrength);
			VT_ADD_INTERNAL_CALL(Material_GetEmissiveStrength);
			VT_ADD_INTERNAL_CALL(Material_SetRoughness);
			VT_ADD_INTERNAL_CALL(Material_GetRoughness);
			VT_ADD_INTERNAL_CALL(Material_SetMetalness);
			VT_ADD_INTERNAL_CALL(Material_GetMetalness);
			VT_ADD_INTERNAL_CALL(Material_SetNormalStrength);
			VT_ADD_INTERNAL_CALL(Material_GetNormalStrength);
		}

		// Post Processing Material
		{
			VT_ADD_INTERNAL_CALL(PostProcessingMaterial_SetBool);
			VT_ADD_INTERNAL_CALL(PostProcessingMaterial_SetInt);
			VT_ADD_INTERNAL_CALL(PostProcessingMaterial_SetUInt);
			VT_ADD_INTERNAL_CALL(PostProcessingMaterial_SetFloat);
			VT_ADD_INTERNAL_CALL(PostProcessingMaterial_SetFloat2);
			VT_ADD_INTERNAL_CALL(PostProcessingMaterial_SetFloat3);
			VT_ADD_INTERNAL_CALL(PostProcessingMaterial_SetFloat4);
		}

		// UIRenderer
		{
			VT_ADD_INTERNAL_CALL(UIRenderer_SetViewport);
			VT_ADD_INTERNAL_CALL(UIRenderer_SetScissor);
			VT_ADD_INTERNAL_CALL(UIRenderer_DrawSprite);
			VT_ADD_INTERNAL_CALL(UIRenderer_DrawSpriteTexture);
			VT_ADD_INTERNAL_CALL(UIRenderer_DrawString);
		}

		// Animation
		{
			VT_ADD_INTERNAL_CALL(Animation_GetDuration);
		}

		// Window
		{
			VT_ADD_INTERNAL_CALL(Window_GetHeight);
			VT_ADD_INTERNAL_CALL(Window_GetWidth);
			VT_ADD_INTERNAL_CALL(Window_SetCursor);
		}

		// Asset
		{
			VT_ADD_INTERNAL_CALL(Asset_IsValid);
		}

		// Steam
		{
			VT_ADD_INTERNAL_CALL(SteamAPI_StartLobby);
			VT_ADD_INTERNAL_CALL(SteamAPI_SetStatInt);
			VT_ADD_INTERNAL_CALL(SteamAPI_SetStatFloat);
			VT_ADD_INTERNAL_CALL(SteamAPI_GetStatInt);
			VT_ADD_INTERNAL_CALL(SteamAPI_GetStatFloat);
			VT_ADD_INTERNAL_CALL(SteamAPI_IsStatsValid);
			VT_ADD_INTERNAL_CALL(SteamAPI_StoreStats);
			VT_ADD_INTERNAL_CALL(SteamAPI_RequestStats);
		}

		// Font
		{
			VT_ADD_INTERNAL_CALL(Font_GetStringWidth);
			VT_ADD_INTERNAL_CALL(Font_GetStringHeight);
		}

		// Post Processing Stack
		{
			VT_ADD_INTERNAL_CALL(PostProcessingStack_PushEffect);
			VT_ADD_INTERNAL_CALL(PostProcessingStack_PopEffect);
		}

		// Renderer
		{
			VT_ADD_INTERNAL_CALL(Renderer_SetRenderScale);
			VT_ADD_INTERNAL_CALL(Renderer_SetRendererSettings);
		}

		// Video Player
		{
			VT_ADD_INTERNAL_CALL(VideoPlayer_Play);
			VT_ADD_INTERNAL_CALL(VideoPlayer_Stop);
			VT_ADD_INTERNAL_CALL(VideoPlayer_Restart);
			VT_ADD_INTERNAL_CALL(VideoPlayer_Pause);
			VT_ADD_INTERNAL_CALL(VideoPlayer_Update);
			VT_ADD_INTERNAL_CALL(VideoPlayer_DrawSpriteTexture);
			VT_ADD_INTERNAL_CALL(VideoPlayer_GetStatus);
		}
	}
}

