#include "vtpch.h"
#include "Volt/Physics/PhysicsActor.h"

#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/PhysXUtilities.h"
#include "Volt/Physics/Physics.h"

#include "Volt/Physics/PhysicsShapes.h"
#include "Volt/Physics/PhysicsLayer.h"

#include "Volt/Components/CoreComponents.h"

namespace Volt
{
	PhysicsActor::PhysicsActor(Entity entity)
		: PhysicsActorBase(entity)
	{
		m_rigidBodyData = myEntity.GetComponent<RigidbodyComponent>();
		CreateRigidActor();
	}

	PhysicsActor::~PhysicsActor()
	{
	}

	void PhysicsActor::SetLinearDrag(float drag) const
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to set drag of non-dynamic PhysicsActor!");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		actor->setLinearDamping(drag);
	}

	void PhysicsActor::SetAngularDrag(float drag) const
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to set drag of non-dynamic PhysicsActor!");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		actor->setAngularDamping(drag);
	}

	void PhysicsActor::SetGravityDisabled(bool disabled) const
	{
		m_rigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, disabled);
	}

	void PhysicsActor::SetMass(float mass)
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Static PhysicsActor can't have mass!");
			return;
		}
		m_rigidBodyData.mass = mass;

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor, mass);
	}

	void PhysicsActor::SetKinematic(bool isKinematic)
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Static PhysicsActor can't be kinematic!");
			return;
		}
		m_rigidBodyData.isKinematic = isKinematic;

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
	}

	void PhysicsActor::SetSimulationData(uint32_t layerId)
	{
		const auto data = PhysXUtilities::CreateFilterDataFromLayer(layerId, (CollisionDetectionType)m_rigidBodyData.collisionType);
		myFilterData = data;

		for (auto& collider : m_colliders)
		{
			collider->SetFilterData(myFilterData);
		}

		myLayerId = layerId;
	}

	void PhysicsActor::SetKinematicTarget(const glm::vec3& position, const glm::quat& rotation)
	{
		if (!IsKinematic())
		{
			VT_LOG(Warning, "Trying to set kinematic target for a non-kinematic actor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");
		actor->setKinematicTarget(PhysXUtilities::ToPhysXTransform(position, rotation));
	}

	void PhysicsActor::SetLinearVelocity(const glm::vec3& velocity)
	{
		if (!IsDynamic() || IsKinematic())
		{
			VT_LOG(Warning, "Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		actor->setLinearVelocity(PhysXUtilities::ToPhysXVector(velocity));
	}

	void PhysicsActor::SetAngularVelocity(const glm::vec3& velocity)
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		actor->setAngularVelocity(PhysXUtilities::ToPhysXVector(velocity));
	}

	void PhysicsActor::SetMaxLinearVelocity(float velocity)
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		actor->setMaxLinearVelocity(velocity);
	}

	void PhysicsActor::SetMaxAngularVelocity(float velocity)
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		actor->setMaxAngularVelocity(velocity);
	}

	const float PhysicsActor::GetMass() const
	{
		return !IsDynamic() ? m_rigidBodyData.mass : m_rigidActor->is<physx::PxRigidDynamic>()->getMass();
	}

	const glm::vec3 PhysicsActor::GetLinearVelocity() const
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to get velocity of non-dynamic PhysicsActor");
			return glm::vec3(0.f);
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		return PhysXUtilities::FromPhysXVector(actor->getLinearVelocity());
	}

	const glm::vec3 PhysicsActor::GetAngularVelocity() const
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to get velocity of non-dynamic PhysicsActor");
			return glm::vec3(0.f);
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		return PhysXUtilities::FromPhysXVector(actor->getAngularVelocity());
	}

	const float PhysicsActor::GetMaxLinearVelocity() const
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to get velocity of non-dynamic PhysicsActor");
			return 0.f;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		return actor->getMaxLinearVelocity();
	}

	const float PhysicsActor::GetMaxAngularVelocity() const
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to get velocity of non-dynamic PhysicsActor");
			return 0.f;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");

		return actor->getMaxAngularVelocity();
	}

	const glm::vec3 PhysicsActor::GetKinematicTargetPosition() const
	{
		if (!IsKinematic())
		{
			VT_LOG(Warning, "Trying to set kinematic target for a non-kinematic actor.");
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");
		physx::PxTransform target;
		actor->getKinematicTarget(target);
		return PhysXUtilities::FromPhysXVector(target.p);
	}

	const glm::quat PhysicsActor::GetKinematicTargetRotation() const
	{
		if (!IsKinematic())
		{
			VT_LOG(Warning, "Trying to set kinematic target for a non-kinematic actor.");
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");
		physx::PxTransform target;
		actor->getKinematicTarget(target);
		return PhysXUtilities::FromPhysXQuat(target.q);
	}

	void PhysicsActor::SetPosition(const glm::vec3& position, bool autoWake, bool synchronize)
	{
		physx::PxTransform transform = m_rigidActor->getGlobalPose();
		transform.p = PhysXUtilities::ToPhysXVector(position);

		m_rigidActor->setGlobalPose(transform);

		if (m_rigidBodyData.bodyType == BodyType::Static && synchronize)
		{
			SynchronizeTransform();
		}

		if (autoWake)
		{
			WakeUp();
		}
	}

	void PhysicsActor::SetRotation(const glm::quat& rotation, bool autoWake, bool synchronize)
	{
		physx::PxTransform transform = m_rigidActor->getGlobalPose();
		transform.q = PhysXUtilities::ToPhysXQuat(rotation);

		m_rigidActor->setGlobalPose(transform);

		if (m_rigidBodyData.bodyType == BodyType::Static && synchronize)
		{
			SynchronizeTransform();
		}

		if (autoWake)
		{
			WakeUp();
		}
	}

	Ref<ColliderShape> PhysicsActor::GetColliderOfType(ColliderType aType) const
	{
		for (const auto& coll : m_colliders)
		{
			if (coll->GetType() == aType)
			{
				return coll;
			}
		}
		return nullptr;
	}

	void PhysicsActor::AddForce(const glm::vec3& aForce, ForceMode aForceMode)
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to add force to non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");
		actor->addForce(PhysXUtilities::ToPhysXVector(aForce), (physx::PxForceMode::Enum)aForceMode);
	}

	void PhysicsActor::AddTorque(const glm::vec3& torque, ForceMode aForceMode)
	{
		if (!IsDynamic())
		{
			VT_LOG(Warning, "Trying to add torque to non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_rigidActor->is<physx::PxRigidDynamic>();
		VT_ASSERT_MSG(actor, "Actor is null!");
		actor->addTorque(PhysXUtilities::ToPhysXVector(torque), (physx::PxForceMode::Enum)aForceMode);
	}

	void PhysicsActor::WakeUp()
	{
		if (IsDynamic())
		{
			m_rigidActor->is<physx::PxRigidDynamic>()->wakeUp();
		}
	}

	void PhysicsActor::PutToSleep()
	{
		if (IsDynamic())
		{
			m_rigidActor->is<physx::PxRigidDynamic>()->putToSleep();
		}
	}

	void PhysicsActor::AddCollider(BoxColliderComponent& collider, Entity entity)
	{
		m_colliders.emplace_back(CreateRef<BoxColliderShape>(collider, *this, entity));
	}

	void PhysicsActor::AddCollider(SphereColliderComponent& collider, Entity entity)
	{
		m_colliders.emplace_back(CreateRef<SphereColliderShape>(collider, *this, entity));
	}

	void PhysicsActor::AddCollider(CapsuleColliderComponent& collider, Entity entity)
	{
		m_colliders.emplace_back(CreateRef<CapsuleColliderShape>(collider, *this, entity));
	}

	void PhysicsActor::AddCollider(MeshColliderComponent& collider, Entity entity)
	{
		if (collider.isConvex)
		{
			m_colliders.emplace_back(CreateRef<ConvexMeshShape>(collider, *this, entity));
		}
		else
		{
			m_colliders.emplace_back(CreateRef<TriangleMeshShape>(collider, *this, entity));
		}
	}

	void PhysicsActor::RemoveCollider(ColliderType type)
	{
		auto it = std::find_if(m_colliders.begin(), m_colliders.end(), [type](Ref<ColliderShape> coll)
		{
			return coll->GetType() == type;
		});

		if (it != m_colliders.end())
		{
			(*it)->DetachFromActor(m_rigidActor);
			(*it)->Release();
			m_colliders.erase(it);
		}
	}

	void PhysicsActor::SetLockFlag(ActorLockFlag flag, bool value, bool forceAwake)
	{
		if (!IsDynamic())
		{
			return;
		}

		if (value)
		{
			m_lockFlags |= (uint32_t)flag;
		}
		else
		{
			m_lockFlags &= ~(uint32_t)flag;
		}

		m_rigidActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags(static_cast<physx::PxRigidDynamicLockFlags>(static_cast<uint8_t>(m_lockFlags)));

		if (forceAwake)
		{
			WakeUp();
		}
	}

	void PhysicsActor::SetLockFlags(uint32_t lockFlags)
	{
		m_lockFlags = lockFlags;

		if (IsDynamic())
		{
			m_rigidActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags(static_cast<physx::PxRigidDynamicLockFlags>(static_cast<uint8_t>(m_lockFlags)));
		}
	}

	void PhysicsActor::CreateRigidActor()
	{
		auto& sdk = PhysXInternal::GetPhysXSDK();
		glm::mat4 transform = { 1 };
		if (myEntity.HasComponent<Volt::TransformComponent>())
		{
			transform = myEntity.GetTransform();
		}

		if (m_rigidBodyData.bodyType == BodyType::Static)
		{
			m_rigidActor = sdk.createRigidStatic(PhysXUtilities::ToPhysXTransform(transform));
		}
		else
		{
			const PhysicsSettings& settings = Physics::GetSettings();
			m_rigidActor = sdk.createRigidDynamic(PhysXUtilities::ToPhysXTransform(transform));

			SetLinearDrag(m_rigidBodyData.linearDrag);
			SetAngularDrag(m_rigidBodyData.angularDrag);
			SetKinematic(m_rigidBodyData.isKinematic);
			SetGravityDisabled(m_rigidBodyData.disableGravity);
			SetLockFlags(m_rigidBodyData.lockFlags);

			m_rigidActor->is<physx::PxRigidDynamic>()->setSolverIterationCounts(settings.solverIterations, settings.solverVelocityIterations);
			m_rigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, m_rigidBodyData.collisionType == CollisionDetectionType::Continuous);
			m_rigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, m_rigidBodyData.collisionType == CollisionDetectionType::ContinuousSpeculative);
		}

		if (myEntity.HasComponent<BoxColliderComponent>()) AddCollider(myEntity.GetComponent<BoxColliderComponent>(), myEntity);
		if (myEntity.HasComponent<SphereColliderComponent>()) AddCollider(myEntity.GetComponent<SphereColliderComponent>(), myEntity);
		if (myEntity.HasComponent<CapsuleColliderComponent>()) AddCollider(myEntity.GetComponent<CapsuleColliderComponent>(), myEntity);
		if (myEntity.HasComponent<MeshColliderComponent>()) AddCollider(myEntity.GetComponent<MeshColliderComponent>(), myEntity);

		if (m_rigidBodyData.bodyType == BodyType::Dynamic)
		{
			SetMass(m_rigidBodyData.mass);
		}

		m_rigidActor->userData = this;

#ifndef VT_DIST
		m_rigidActor->setName(myEntity.GetTag().c_str());
#endif

		SetSimulationData(m_rigidBodyData.layerId);
	}

	void PhysicsActor::SynchronizeTransform()
	{
		if (m_toBeRemoved)
		{ 
			return;
		}

		if (!myEntity.HasComponent<TransformComponent>()) 
		{
			return; 
		}

		physx::PxTransform actorPose = m_rigidActor->getGlobalPose();

		myEntity.SetPosition(PhysXUtilities::FromPhysXVector(actorPose.p), false);

		if (!IsAllRotationLocked())
		{
			myEntity.SetRotation(PhysXUtilities::FromPhysXQuat(actorPose.q), false);
		}
	}
}
