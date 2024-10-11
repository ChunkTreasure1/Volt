#pragma once

#include "Volt/Physics/PhysicsEnums.h"

#include <EntitySystem/Scripting/ECSAccessBuilder.h>

namespace Volt
{
	namespace Internal
	{
		struct RigidbodyComponentInternal_BodyTypeUpdated
		{
			BodyType bodyType;
		};

		struct RigidbodyComponentInternal_LayerIdUpdated
		{
			uint32_t layerId;
		};

		struct RigidbodyComponentInternal_MassUpdated
		{
			float mass;
		};

		struct RigidbodyComponentInternal_LinearDragUpdated
		{
			float linearDrag;
		};

		struct RigidbodyComponentInternal_LockFlagsUpdated
		{
			uint32_t lockFlags;
		};

		struct RigidbodyComponentInternal_AngularDragUpdated
		{
			float angularDrag;
		};

		struct RigidbodyComponentInternal_DisableGravityUpdated
		{
			bool disableGravity;
		};

		struct RigidbodyComponentInternal_IsKinematicUpdated
		{
			bool isKinematic;
		};

		struct RigidbodyComponentInternal_SetKinematicTarget
		{
			glm::vec3 translation;
			glm::quat rotation;
		};

		struct RigidbodyComponentInternal_SetLinearVelocity
		{
			glm::vec3 velocity;
		};

		struct RigidbodyComponentInternal_SetAngularVelocity
		{
			glm::vec3 velocity;
		};

		struct RigidbodyComponentInternal_SetMaxLinearVelocity
		{
			float velocity;
		};

		struct RigidbodyComponentInternal_SetMaxAngularVelocity
		{
			float velocity;
		};

		struct RigidbodyComponentInternal_AddForce_Force
		{
			glm::vec3 force = 0.f;
		};

		struct RigidbodyComponentInternal_AddForce_Impulse
		{
			glm::vec3 force = 0.f;
		};

		struct RigidbodyComponentInternal_AddForce_VelocityChange
		{
			glm::vec3 force = 0.f;
		};

		struct RigidbodyComponentInternal_AddForce_Acceleration
		{
			glm::vec3 force = 0.f;
		};

		struct RigidbodyComponentInternal_AddTorque_Force
		{
			glm::vec3 torque = 0.f;
		};

		struct RigidbodyComponentInternal_AddTorque_Impulse
		{
			glm::vec3 torque = 0.f;
		};

		struct RigidbodyComponentInternal_AddTorque_VelocityChange
		{
			glm::vec3 torque = 0.f;
		};

		struct RigidbodyComponentInternal_AddTorque_Acceleration
		{
			glm::vec3 torque = 0.f;
		};

		struct RigidbodyComponentInternal_WakeUp
		{
		};

		struct RigidbodyComponentInternal_PutToSleep
		{
		};
	}
	
	struct RigidbodyComponent
	{
		// #TODO_Ivar: Make private
		BodyType m_bodyType = BodyType::Static;
		uint32_t m_layerId = 0;
		float m_mass = 1.f;
		float m_linearDrag = 0.01f;
		uint32_t m_lockFlags = 0; // #TODO: add enum support
		float m_angularDrag = 0.05f;
		CollisionDetectionType m_collisionType = CollisionDetectionType::Discrete;

		bool m_disableGravity = false;
		bool m_isKinematic = false;

		inline RigidbodyComponent(BodyType aBodyType = BodyType::Static, uint32_t aLayerId = 0, float aMass = 1.f, float aLinearDrag = 0.01f, uint32_t aLockFlags = 0,
			float aAngularDrag = 0.05f, bool aDisableGravity = false, bool aIsKinematic = false, CollisionDetectionType aCollisionType = CollisionDetectionType::Discrete)
			: m_bodyType(aBodyType), m_layerId(aLayerId), m_mass(aMass), m_linearDrag(aLinearDrag), m_lockFlags(aLockFlags), m_angularDrag(aAngularDrag), m_collisionType(aCollisionType),
			m_disableGravity(aDisableGravity), m_isKinematic(aIsKinematic)
		{
		}

		VT_NODISCARD VT_INLINE BodyType GetBodyType() const { return m_bodyType; }
		VT_NODISCARD VT_INLINE uint32_t GetLayerId() const { return m_layerId; }
		VT_NODISCARD VT_INLINE float GetMass() const { return m_mass; }
		VT_NODISCARD VT_INLINE float GetLinearDrag() const { return m_linearDrag; }
		VT_NODISCARD VT_INLINE uint32_t GetLockFlags() const { return m_lockFlags; }
		VT_NODISCARD VT_INLINE float GetAngularDrag() const { return m_angularDrag; }
		VT_NODISCARD VT_INLINE CollisionDetectionType GetCollisionDetectionType() const { return m_collisionType; }
		VT_NODISCARD VT_INLINE bool GetDisableGravity() const { return m_disableGravity; }
		VT_NODISCARD VT_INLINE bool GetIsKinematic() const { return m_isKinematic; }

		template<typename EntityType> void SetBodyType(EntityType& entity, BodyType bodyType);
		template<typename EntityType> void SetLayerId(EntityType& entity, uint32_t layerId);
		template<typename EntityType> void SetMass(EntityType& entity, float mass);
		template<typename EntityType> void SetLinearDrag(EntityType& entity, float linearDrag);
		template<typename EntityType> void SetLockFlags(EntityType& entity, uint32_t lockFlags);
		template<typename EntityType> void SetAngularDrag(EntityType& entity, float angularDrag);
		template<typename EntityType> void SetCollisionDetectionType(EntityType& entity, CollisionDetectionType detectionType);
		template<typename EntityType> void SetDisableGravity(EntityType& entity, bool state);
		template<typename EntityType> void SetIsKinematic(EntityType& entity, bool state);
		template<typename EntityType> void SetKinematicTarget(EntityType& entity, const glm::vec3& translation, const glm::quat& rotation);
		template<typename EntityType> void SetLinearVelocity(EntityType& entity, const glm::vec3& velocity);
		template<typename EntityType> void SetAngularVelocity(EntityType& entity, const glm::vec3& velocity);
		template<typename EntityType> void SetMaxLinearVelocity(EntityType& entity, float velocity);
		template<typename EntityType> void SetMaxAngularVelocity(EntityType& entity, float velocity);
		template<typename EntityType> void AddForce(EntityType& entity, const glm::vec3& force, ForceMode forceMode);
		template<typename EntityType> void AddTorque(EntityType& entity, const glm::vec3& torque, ForceMode forceMode);
		template<typename EntityType> void WakeUp(EntityType& entity);
		template<typename EntityType> void PutToSleep(EntityType& entity);

		static void ReflectType(TypeDesc<RigidbodyComponent>& reflect)
		{
			reflect.SetGUID("{460B7722-00C0-48BE-8B3E-B549BCC9269B}"_guid);
			reflect.SetLabel("Rigidbody Component");
			reflect.AddMember(&RigidbodyComponent::m_bodyType, "bodyType", "Body Type", "", BodyType::Static);
			reflect.AddMember(&RigidbodyComponent::m_layerId, "layerId", "Layer ID", "", 0);
			reflect.AddMember(&RigidbodyComponent::m_mass, "mass", "Mass", "", 1.f);
			reflect.AddMember(&RigidbodyComponent::m_linearDrag, "linearDrag", "Linear Drag", "", 0.01f);
			reflect.AddMember(&RigidbodyComponent::m_lockFlags, "lockFlags", "Lock Flags", "", 0);
			reflect.AddMember(&RigidbodyComponent::m_angularDrag, "angularDrag", "Angular Drag", "", 0.05f);
			reflect.AddMember(&RigidbodyComponent::m_collisionType, "collisionType", "Collision Type", "", CollisionDetectionType::Discrete);
			reflect.AddMember(&RigidbodyComponent::m_disableGravity, "disableGravity", "Disable Gravity", "", false);
			reflect.AddMember(&RigidbodyComponent::m_isKinematic, "isKinematic", "Is Kinematic", "", false);
			reflect.SetOnCreateCallback(&RigidbodyComponent::OnCreate);
			reflect.SetOnDestroyCallback(&RigidbodyComponent::OnDestroy);
			reflect.SetOnTransformChangedCallback(&RigidbodyComponent::OnTransformChanged);
		}

		REGISTER_COMPONENT(RigidbodyComponent);

	private:
		using PhysicsEntity = ECS::Access
			::Read<RigidbodyComponent>
			::As<ECS::Type::Entity>;

		using PhysicsTransformEntity = ECS::Access
			::Read<TransformComponent>
			::With<RigidbodyComponent>
			::As<ECS::Type::Entity>;

		static void OnCreate(PhysicsEntity entity);
		static void OnDestroy(PhysicsEntity entity);
		static void OnTransformChanged(PhysicsTransformEntity entity);
	};

	template<typename T, typename EntityType>
	inline T& GetOrCreateComponent(EntityType& entity)
	{
		if (!entity.HasComponent<T>())
		{
			return entity.AddComponent<T>();
		}

		return entity.GetComponentUnsafe<T>();
	}

	template<typename EntityType>
	inline void RigidbodyComponent::SetBodyType(EntityType& entity, BodyType bodyType)
	{
		m_bodyType = bodyType;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_BodyTypeUpdated>(entity).bodyType = bodyType;
	}

	template<typename EntityType>
	inline void RigidbodyComponent::SetLayerId(EntityType& entity, uint32_t layerId)
	{
		m_layerId = layerId;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_LayerIdUpdated>(entity).layerId = layerId;
	}

	template<typename EntityType>
	inline void RigidbodyComponent::SetMass(EntityType& entity, float mass)
	{
		m_mass = mass;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_MassUpdated>(entity).mass = mass;
	}

	template<typename EntityType>
	inline void RigidbodyComponent::SetLinearDrag(EntityType& entity, float linearDrag)
	{
		m_linearDrag = linearDrag;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_LinearDragUpdated>(entity).linearDrag = linearDrag;
	}

	template<typename EntityType>
	inline void RigidbodyComponent::SetLockFlags(EntityType& entity, uint32_t lockFlags)
	{
		m_lockFlags = lockFlags;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_LockFlagsUpdated>(entity).lockFlags = lockFlags;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetAngularDrag(EntityType& entity, float angularDrag)
	{
		m_angularDrag = angularDrag;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_AngularDragUpdated>(entity).angularDrag = angularDrag;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetCollisionDetectionType(EntityType& entity, CollisionDetectionType detectionType)
	{
		m_collisionType = detectionType;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetDisableGravity(EntityType& entity, bool state)
	{
		m_disableGravity = state;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_DisableGravityUpdated>(entity).disableGravity = state;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetIsKinematic(EntityType& entity, bool state)
	{
		m_isKinematic = state;
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_IsKinematicUpdated>(entity).isKinematic = state;
	}

	template<typename EntityType>
	inline void RigidbodyComponent::SetKinematicTarget(EntityType& entity, const glm::vec3& translation, const glm::quat& rotation)
	{
		auto& comp = GetOrCreateComponent<Internal::RigidbodyComponentInternal_SetKinematicTarget>(entity);
		comp.translation = translation;
		comp.rotation = rotation;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetLinearVelocity(EntityType& entity, const glm::vec3& velocity)
	{
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_SetLinearVelocity>(entity).velocity = velocity;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetAngularVelocity(EntityType& entity, const glm::vec3& velocity)
	{
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_SetAngularVelocity>(entity).velocity = velocity;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetMaxLinearVelocity(EntityType& entity, float velocity)
	{
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_SetMaxLinearVelocity>(entity).velocity = velocity;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::SetMaxAngularVelocity(EntityType& entity, float velocity)
	{
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_SetMaxAngularVelocity>(entity).velocity = velocity;
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::AddForce(EntityType& entity, const glm::vec3& force, ForceMode forceMode)
	{
		if (forceMode == ForceMode::Force)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddForce_Force>(entity).force += force;
		}
		else if (forceMode == ForceMode::Impulse)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddForce_Impulse>(entity).force += force;
		}
		else if (forceMode == ForceMode::VelocityChange)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddForce_VelocityChange>(entity).force += force;
		}
		else if (forceMode == ForceMode::Acceleration)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddForce_Acceleration>(entity).force += force;
		}
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::AddTorque(EntityType& entity, const glm::vec3& torque, ForceMode forceMode)
	{
		if (forceMode == ForceMode::Force)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddTorque_Force>(entity).torque += torque;
		}
		else if (forceMode == ForceMode::Impulse)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddTorque_Impulse>(entity).torque += torque;
		}
		else if (forceMode == ForceMode::VelocityChange)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddTorque_VelocityChange>(entity).torque += torque;
		}
		else if (forceMode == ForceMode::Acceleration)
		{
			GetOrCreateComponent<Internal::RigidbodyComponentInternal_AddTorque_Acceleration>(entity).torque += torque;
		}
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::WakeUp(EntityType& entity)
	{
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_WakeUp>(entity);
	}
	
	template<typename EntityType>
	inline void RigidbodyComponent::PutToSleep(EntityType& entity)
	{
		GetOrCreateComponent<Internal::RigidbodyComponentInternal_PutToSleep>(entity);
	}
}
