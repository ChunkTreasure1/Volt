#pragma once

#include "Volt/Scene/Entity.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Physics/PhysicsEnums.h"
#include "Volt/Physics/PhysicsActorBase.h"


#include <PhysX/PxPhysics.h>
#include <PhysX/PxFiltering.h>

namespace Volt
{
	class ColliderShape;
	class PhysicsActor : public PhysicsActorBase
	{
	public:
		PhysicsActor(Entity entity);
		~PhysicsActor();

		void SetLinearDrag(float drag) const;
		void SetAngularDrag(float drag) const;

		void SetGravityDisabled(bool disabled) const;
		void SetMass(float mass);

		void SetKinematic(bool isKinematic);
		void SetSimulationData(uint32_t layerId) override;

		void SetKinematicTarget(const glm::vec3& position, const glm::quat& rotation);
		void SetLinearVelocity(const glm::vec3& velocity);
		void SetAngularVelocity(const glm::vec3& velocity);

		void SetMaxLinearVelocity(float velocity);
		void SetMaxAngularVelocity(float velocity);

		const float GetMass() const;
		const glm::vec3 GetLinearVelocity() const;
		const glm::vec3 GetAngularVelocity() const;

		const float GetMaxLinearVelocity() const;
		const float GetMaxAngularVelocity() const;

		const glm::vec3 GetKinematicTargetPosition() const;
		const glm::quat GetKinematicTargetRotation() const;

		void SetPosition(const glm::vec3& position, bool autoWake = true, bool synchronize = true);
		void SetRotation(const glm::quat& rotation, bool autoWake = true, bool synchronize = true);

		Ref<ColliderShape> GetColliderOfType(ColliderType aType) const;

		void AddForce(const glm::vec3& aForce, ForceMode aForceMode);
		void AddTorque(const glm::vec3& torque, ForceMode aForceMode);

		void WakeUp();
		void PutToSleep();

		inline physx::PxRigidActor& GetActor() const { return *myRigidActor; }
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

	protected:
		void SynchronizeTransform() override;

	private:
		friend class PhysicsScene;

		void CreateRigidActor();

		bool myToBeRemoved = false;

		RigidbodyComponent myRigidBodyData;

		uint32_t myLockFlags = 0;
		physx::PxRigidActor* myRigidActor;

		std::vector<Ref<ColliderShape>> myColliders;
	};
}
