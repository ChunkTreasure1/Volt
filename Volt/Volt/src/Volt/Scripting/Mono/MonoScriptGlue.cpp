#include "vtpch.h"
#include "MonoScriptGlue.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"

#include "Volt/Input/Input.h"
#include <Volt/Components/Components.h>
#include <Volt/Components/PhysicsComponents.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Volt
{
#define VT_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volt.InternalCalls::" #Name, Name)

#pragma region Entity
	inline static bool Entity_HasComponent(Wire::EntityId entityId, MonoString* componentType)
	{
		Scene* scene = MonoScriptEngine::GetSceneContext();
		std::string name = mono_string_to_utf8(componentType);

		return scene->GetRegistry().HasComponent(Wire::ComponentRegistry::GetRegistryDataFromName(name).guid, entityId);
	}
#pragma endregion Entity

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
#pragma endregion TransformComponent

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
#pragma endregion TagComponent

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
#pragma endregion RelationshipComponent

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

	void MonoScriptGlue::RegisterFunctions()
	{
		// Entity
		{
			VT_ADD_INTERNAL_CALL(Entity_HasComponent);
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

