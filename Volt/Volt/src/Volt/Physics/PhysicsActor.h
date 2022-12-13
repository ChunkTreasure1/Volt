#pragma once

#include "Volt/Scene/Entity.h"
#include "Volt/Components/PhysicsComponents.h"
#include "Volt/Physics/PhysicsEnums.h"

#include <PhysX/PxPhysics.h>
#include <PhysX/PxFiltering.h>

namespace Volt
{
	class ColliderShape;
	class PhysicsActor
	{
	public:
		PhysicsActor(Entity entity);
		~PhysicsActor();

		void SetLinearDrag(float drag) const;
		void SetAngularDrag(float drag) const;
		
		void SetGravityDisabled(bool disabled) const;
		void SetMass(float mass);
		
		void SetKinematic(bool isKinematic);
		void SetSimulationData(uint32_t layerId);
		
		void SetKinematicTarget(const gem::vec3& position, const gem::quat& rotation);
		void SetLinearVelocity(const gem::vec3& velocity);
		void SetAngularVelocity(const gem::vec3& velocity);

		void SetMaxLinearVelocity(float velocity);
		void SetMaxAngularVelocity(float velocity);

		const float GetMass() const;
		const gem::vec3 GetLinearVelocity() const;
		const gem::vec3 GetAngularVelocity() const;

		const float GetMaxLinearVelocity() const;
		const float GetMaxAngularVelocity() const;

		const gem::vec3 GetKinematicTargetPosition() const;
		const gem::quat GetKinematicTargetRotation() const;

		void SetPosition(const gem::vec3& position, bool autoWake = true);
		void SetRotation(const gem::quat& rotation, bool autoWake = true);

		Ref<ColliderShape> GetColliderOfType(ColliderType aType) const;

		void AddForce(const gem::vec3& aForce, ForceMode aForceMode);
		void AddTorque(const gem::vec3& torque, ForceMode aForceMode);
		
		void WakeUp();
		void PutToSleep();

		inline const Entity GetEntity() const { return myEntity; }
		inline physx::PxRigidActor& GetActor() const { return *myRigidActor; }
		inline const physx::PxFilterData& GetFilterData() const { return myFilterData; }
		inline const RigidbodyComponent& GetRigidbodyData() const { return myRigidBodyData; }

		inline const bool IsDynamic() const { return myRigidBodyData.bodyType == BodyType::Dynamic; }
		inline bool IsKinematic() const { return IsDynamic() && myRigidBodyData.isKinematic; }

		inline const bool IsLockFlagSet(ActorLockFlag flag) const { return (uint32_t)flag & myLockFlags; }
		inline const uint32_t GetLockFlags() const { return myLockFlags; }
		inline const bool IsAllRotationLocked() const { return IsLockFlagSet(ActorLockFlag::RotationX) && IsLockFlagSet(ActorLockFlag::RotationY) && IsLockFlagSet(ActorLockFlag::RotationZ); }

		void AddCollider(BoxColliderComponent& collider, Entity entity);
		void AddCollider(SphereColliderComponent& collider, Entity entity);
		void AddCollider(CapsuleColliderComponent& collider, Entity entity);
		void AddCollider(MeshColliderComponent& collider, Entity entity);

		void RemoveCollider(ColliderType type);

		void SetLockFlag(ActorLockFlag flag, bool value, bool forceAwake = false);
		void SetLockFlags(uint32_t lockFlags);

	private:
		friend class PhysicsScene;
		
		void SynchronizeTransform();
		void CreateRigidActor();

		bool myToBeRemoved = false;

		Entity myEntity;
		RigidbodyComponent myRigidBodyData;

		uint32_t myLockFlags = 0;
		physx::PxRigidActor* myRigidActor;
		physx::PxFilterData myFilterData;

		std::vector<Ref<ColliderShape>> myColliders;
	};
}