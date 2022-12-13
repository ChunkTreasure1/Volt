#include "vtpch.h"
#include "MonoScriptGlue.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"

#include "Volt/Input/Input.h"
#include "Volt/Components/Components.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Physics/PhysicsActor.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Volt
{
#define VT_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volt.InternalCalls::" #Name, Name)

#pragma region Entity
	inline static bool Entity_HasComponent(Wire::EntityId entityId, MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		char* cStr = mono_string_to_utf8(componentType);
		std::string compName(cStr);
		
		mono_free(cStr);

		return scene->GetRegistry().HasComponent(Wire::ComponentRegistry::GetRegistryDataFromName(compName).guid, entityId);
	}

	inline static void Entity_RemoveComponent(Wire::EntityId entityId, MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		char* cStr = mono_string_to_utf8(componentType);
		std::string compName(cStr);

		mono_free(cStr);

		scene->GetRegistry().RemoveComponent(Wire::ComponentRegistry::GetRegistryDataFromName(compName).guid, entityId);
	}

	inline static void Entity_AddComponent(Wire::EntityId entityId, MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();

		char* cStr = mono_string_to_utf8(componentType);
		std::string compName(cStr);

		mono_free(cStr);

		scene->GetRegistry().AddComponent(Wire::ComponentRegistry::GetRegistryDataFromName(compName).guid, entityId);
	}
#pragma endregion

#pragma region TransformComponent
	inline static void TransformComponent_GetPosition(Wire::EntityId entityId, gem::vec3* outPosition)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		*outPosition = entity.GetWorldPosition();
	}

	inline static void TransformComponent_SetPosition(Wire::EntityId entityId, gem::vec3* translation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		entity.SetWorldPosition(*translation);
	}

	inline static void TransformComponent_GetRotation(Wire::EntityId entityId, gem::vec3* outRotation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		*outRotation = entity.GetWorldRotation();
	}

	inline static void TransformComponent_SetRotation(Wire::EntityId entityId, gem::vec3* rotation)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		entity.SetRotation(*rotation);
	}

	inline static void TransformComponent_GetScale(Wire::EntityId entityId, gem::vec3* outScale)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		*outScale = entity.GetWorldScale();
	}

	inline static void TransformComponent_SetScale(Wire::EntityId entityId, gem::vec3* scale)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		entity.SetScale(*scale);
	}

	inline static void TransformComponent_GetForward(Wire::EntityId entityId, gem::vec3* outForward)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		*outForward = entity.GetWorldForward();
	}

	inline static void TransformComponent_GetRight(Wire::EntityId entityId, gem::vec3* outRight)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		*outRight = entity.GetWorldRight();
	}

	inline static void TransformComponent_GetUp(Wire::EntityId entityId, gem::vec3* outUp)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		*outUp = entity.GetWorldUp();
	}
#pragma endregion

#pragma region TagComponent
	inline static void TagComponent_SetTag(Wire::EntityId entityId, MonoString* tag)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		char* cStr = mono_string_to_utf8(tag);
		std::string str(cStr);
		mono_free(cStr);

		entity.GetComponent<TagComponent>().tag = str;
	}

	inline static void TagComponent_GetTag(Wire::EntityId entityId, MonoString* outString)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		outString = mono_string_new(MonoScriptEngine::GetAppDomain(), entity.GetComponent<TagComponent>().tag.c_str());
	}
#pragma endregion

#pragma region Log
	inline static void Log_String(MonoString* string, LogLevel logLevel)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);

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
	inline static void RelationshipComponent_GetChildren(Wire::EntityId entityId, MonoArray* outChildren)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		MonoArray* array = mono_array_new(MonoScriptEngine::GetAppDomain(), MonoScriptEngine::GetEntityMonoClass()->GetClass(), (uint32_t)entity.GetChilden().size());

		//for (const auto& child : entity.GetChilden())
		//{
		//	mono_array_set(array, )
		//}
	}

	inline static Wire::EntityId RelationshipComponent_GetParent(Wire::EntityId entityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		return entity.GetParent().GetId();
	}

	inline static void RelationshipComponent_SetParent(Wire::EntityId entityId, Wire::EntityId* parentEntityId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.GetParent())
		{
			scene->UnparentEntity(entity);
		}

		if (scene->GetRegistry().Exists(*parentEntityId))
		{
			scene->ParentEntity(Volt::Entity{ *parentEntityId, scene }, entity);
		}
	}
#pragma endregion

#pragma region RigidbodyComponent
	inline static BodyType RigidbodyComponent_GetBodyType(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetBodyType(Wire::EntityId entityId, BodyType* bodyType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().bodyType = *bodyType;
		}
	}

	inline static uint32_t RigidbodyComponent_GetLayerId(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetLayerId(Wire::EntityId entityId, uint32_t* layerId)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().layerId = *layerId;
		}
	}

	inline static float RigidbodyComponent_GetMass(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetMass(Wire::EntityId entityId, float* mass)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().mass = *mass;
		}
	}

	inline static void RigidbodyComponent_SetLinearDrag(Wire::EntityId entityId, float* linearDrag)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().linearDrag = *linearDrag;
		}
	}

	inline static float RigidbodyComponent_GetLinearDrag(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetAngularDrag(Wire::EntityId entityId, float* angularDrag)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().angularDrag = *angularDrag;
		}
	}

	inline static float RigidbodyComponent_GetAngularDrag(Wire::EntityId entityId)
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

	inline static uint32_t RigidbodyComponent_GetLockFlags(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetLockFlags(Wire::EntityId entityId, uint32_t* flags)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().lockFlags = *flags;
		}
	}

	inline static bool RigidbodyComponent_GetDisableGravity(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetDisableGravity(Wire::EntityId entityId, bool* state)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().disableGravity = *state;
		}
	}

	inline static bool RigidbodyComponent_GetIsKinematic(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetIsKinematic(Wire::EntityId entityId, bool* state)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().isKinematic = *state;
		}
	}

	inline static CollisionDetectionType RigidbodyComponent_GetCollisionDetectionType(Wire::EntityId entityId)
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

	inline static void RigidbodyComponent_SetCollisionDetectionType(Wire::EntityId entityId, CollisionDetectionType* collisionDetectionType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Volt::Entity entity{ entityId, scene };

		if (entity.HasComponent<RigidbodyComponent>())
		{
			entity.GetComponent<RigidbodyComponent>().collisionType = *collisionDetectionType;
		}
	}
#pragma endregion

#pragma region BoxColliderComponent
	inline static void BoxColliderComponent_GetHalfSize(Wire::EntityId entityId, gem::vec3* outHalfSize)
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

	inline static void BoxColliderComponent_SetHalfSize(Wire::EntityId entityId, gem::vec3* halfSize)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			entity.GetComponent<BoxColliderComponent>().halfSize = *halfSize;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
		}
	}

	inline static void BoxColliderComponent_GetOffset(Wire::EntityId entityId, gem::vec3* outOffset)
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

	inline static void BoxColliderComponent_SetOffset(Wire::EntityId entityId, gem::vec3* offset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			entity.GetComponent<BoxColliderComponent>().offset = *offset;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
		}
	}

	inline static bool BoxColliderComponent_GetIsTrigger(Wire::EntityId entityId)
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

	inline static void BoxColliderComponent_SetIsTrigger(Wire::EntityId entityId, bool* isTrigger)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<BoxColliderComponent>())
		{
			entity.GetComponent<BoxColliderComponent>().isTrigger = *isTrigger;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a BoxColliderComponent!", entityId);
		}
	}
#pragma endregion

#pragma region SphereColliderComponent
	inline static float SphereColliderComponent_GetRadius(Wire::EntityId entityId)
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

	inline static void SphereColliderComponent_SetRadius(Wire::EntityId entityId, float* radius)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			entity.GetComponent<SphereColliderComponent>().radius = *radius;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		}
	}

	inline static void SphereColliderComponent_GetOffset(Wire::EntityId entityId, gem::vec3* outOffset)
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

	inline static void SphereColliderComponent_SetOffset(Wire::EntityId entityId, gem::vec3* offset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			entity.GetComponent<SphereColliderComponent>().offset = *offset;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		}
	}

	inline static bool SphereColliderComponent_GetIsTrigger(Wire::EntityId entityId)
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

	inline static void SphereColliderComponent_SetIsTrigger(Wire::EntityId entityId, bool* isTrigger)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<SphereColliderComponent>())
		{
			entity.GetComponent<SphereColliderComponent>().isTrigger = *isTrigger;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a SphereColliderComponent!", entityId);
		}
	}
#pragma endregion

#pragma region CapsuleColliderComponent
	inline static float CapsuleColliderComponent_GetRadius(Wire::EntityId entityId)
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

	inline static void CapsuleColliderComponent_SetRadius(Wire::EntityId entityId, float* radius)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().radius = *radius;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}

	inline static float CapsuleColliderComponent_GetHeight(Wire::EntityId entityId)
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

	inline static void CapsuleColliderComponent_SetHeight(Wire::EntityId entityId, float* height)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().height = *height;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}

	inline static void CapsuleColliderComponent_GetOffset(Wire::EntityId entityId, gem::vec3* outOffset)
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

	inline static void CapsuleColliderComponent_SetOffset(Wire::EntityId entityId, gem::vec3* offset)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene, };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().offset = *offset;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}

	inline static bool CapsuleColliderComponent_GetIsTrigger(Wire::EntityId entityId)
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

	inline static void CapsuleColliderComponent_SetIsTrigger(Wire::EntityId entityId, bool* isTrigger)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		Entity entity{ entityId, scene };
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			entity.GetComponent<CapsuleColliderComponent>().isTrigger = *isTrigger;
		}
		else
		{
			VT_CORE_ERROR("Entity {0} does not have a CapsuleColliderComponent!", entityId);
		}
	}
#pragma endregion

#pragma region MeshColliderComponent
	inline static bool MeshColliderComponent_GetIsConvex(Wire::EntityId entityId)
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

	inline static void MeshColliderComponent_SetIsConvex(Wire::EntityId entityId, bool* isConvex)
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

	inline static bool MeshColliderComponent_GetIsTrigger(Wire::EntityId entityId)
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

	inline static void MeshColliderComponent_SetIsTrigger(Wire::EntityId entityId, bool* isTrigger)
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

	inline static int32_t MeshColliderComponent_GetSubMeshIndex(Wire::EntityId entityId)
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

	inline static void MeshColliderComponent_SetSubMeshIndex(Wire::EntityId entityId, int32_t* subMeshIndex)
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
#pragma endregion

#pragma region Input
	inline static bool Input_KeyDown(int32_t keyCode)
	{
		return Input::IsKeyDown(keyCode);
	}

	inline static bool Input_KeyUp(int32_t keyCode)
	{
		return Input::IsKeyUp(keyCode);
	}

	inline static bool Input_MouseDown(int32_t button)
	{
		return Input::IsMouseButtonPressed(button);
	}

	inline static bool Input_MouseUp(int32_t button)
	{
		return Input::IsMouseButtonReleased(button);
	}

	inline static void Input_SetMousePosition(float x, float y)
	{
		Input::SetMousePosition(x, y);
	}

	inline static bool Input_GetMousePosition()
	{
		return false;
	}
#pragma endregion

#pragma region PhysicsActor
	inline static void PhysicsActor_SetKinematicTarget(Wire::EntityId entityId, gem::vec3* position, gem::quat* rotation)
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

		actor->SetKinematicTarget(*position, gem::eulerAngles(*rotation));
	}

	inline static void PhysicsActor_SetLinearVelocity(Wire::EntityId entityId, gem::vec3* velocity)
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

	inline static void PhysicsActor_SetAngularVelocity(Wire::EntityId entityId, gem::vec3* velocity)
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

	inline static void PhysicsActor_SetMaxLinearVelocity(Wire::EntityId entityId, float* velocity)
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

	inline static void PhysicsActor_SetMaxAngularVelocity(Wire::EntityId entityId, float* velocity)
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

	inline static void PhysicsActor_GetKinematicTargetPosition(Wire::EntityId entityId, gem::vec3* outPosition)
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

	inline static void PhysicsActor_GetKinematicTargetRotation(Wire::EntityId entityId, gem::quat* outRotation)
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

	inline static void PhysicsActor_AddForce(Wire::EntityId entityId, gem::vec3* force, ForceMode forceMode)
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

	inline static void PhysicsActor_AddTorque(Wire::EntityId entityId, gem::vec3* torque, ForceMode forceMode)
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

	inline static void PhysicsActor_WakeUp(Wire::EntityId entityId)
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

	inline static void PhysicsActor_PutToSleep(Wire::EntityId entityId)
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

	void MonoScriptGlue::RegisterFunctions()
	{
		// Entity
		{
			VT_ADD_INTERNAL_CALL(Entity_HasComponent);
			VT_ADD_INTERNAL_CALL(Entity_RemoveComponent);
			VT_ADD_INTERNAL_CALL(Entity_AddComponent);
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
		}

		// Tag Component
		{
			VT_ADD_INTERNAL_CALL(TagComponent_SetTag);
			VT_ADD_INTERNAL_CALL(TagComponent_GetTag);
		}

		// Relationship Component
		{
			VT_ADD_INTERNAL_CALL(RelationshipComponent_SetParent);
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
		}

		// Physics Actor
		{
			VT_ADD_INTERNAL_CALL(PhysicsActor_SetKinematicTarget);
			
			VT_ADD_INTERNAL_CALL(PhysicsActor_SetLinearVelocity);
			VT_ADD_INTERNAL_CALL(PhysicsActor_SetMaxLinearVelocity);

			VT_ADD_INTERNAL_CALL(PhysicsActor_SetAngularVelocity);
			VT_ADD_INTERNAL_CALL(PhysicsActor_SetMaxAngularVelocity);
		
			VT_ADD_INTERNAL_CALL(PhysicsActor_GetKinematicTargetPosition);
			VT_ADD_INTERNAL_CALL(PhysicsActor_GetKinematicTargetRotation);

			VT_ADD_INTERNAL_CALL(PhysicsActor_AddForce);
			VT_ADD_INTERNAL_CALL(PhysicsActor_AddTorque);

			VT_ADD_INTERNAL_CALL(PhysicsActor_WakeUp);
			VT_ADD_INTERNAL_CALL(PhysicsActor_PutToSleep);
		}

		// Input
		{
			VT_ADD_INTERNAL_CALL(Input_KeyDown);
			VT_ADD_INTERNAL_CALL(Input_KeyUp);
			VT_ADD_INTERNAL_CALL(Input_MouseDown);
			VT_ADD_INTERNAL_CALL(Input_MouseUp);
			VT_ADD_INTERNAL_CALL(Input_SetMousePosition);
			VT_ADD_INTERNAL_CALL(Input_GetMousePosition);
		}
	}
}

