#include "vtpch.h"
#include "PhysicsControllerActor.h"

#include "Volt/Physics/ContactListener.h"
#include "Volt/Physics/PhysXUtilities.h"
#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/Physics.h"

#include <PhysX/PxPhysics.h>
#include <PhysX/characterkinematic/PxController.h>
#include <PhysX/characterkinematic/PxCapsuleController.h>
#include <PhysX/characterkinematic/PxBoxController.h>


namespace Volt
{
	static CharacterControllerContactListener s_contactListener;

	PhysicsControllerActor::PhysicsControllerActor(Entity entity)
		: PhysicsActorBase(entity), myGravity(glm::length(Physics::GetSettings().gravity))
	{
	}

	PhysicsControllerActor::~PhysicsControllerActor()
	{
		if (myController)
		{
			myController->release();
		}

		if (myMaterial)
		{
			myMaterial->release();
		}
	}

	void PhysicsControllerActor::Create(physx::PxControllerManager* controllerManager)
	{
		CreateController(controllerManager);
	}

	void PhysicsControllerActor::Update(float deltaTime)
	{
		if (myController == nullptr) return;

		if (myHasGravity)
		{
			myDownSpeed += myGravity * deltaTime;
		}

		physx::PxControllerFilters filters{};
		filters.mCCTFilterCallback = nullptr;
		filters.mFilterCallback = &s_contactListener;
		filters.mFilterData = &myFilterData;

		glm::vec3 movement = myMovement - glm::vec3{ 0.f, 1.f, 0.f } * myDownSpeed * deltaTime;
		myCurrentCollisionFlags = myController->move(PhysXUtilities::ToPhysXVector(movement), 0.f, deltaTime, filters);

		if (IsGrounded())
		{
			myDownSpeed = myGravity * 0.01f;
		}

		myMovement = 0.f;
	}

	const float PhysicsControllerActor::GetHeight() const
	{
		return 0.f;
	}

	const float PhysicsControllerActor::GetRadius() const
	{
		return 0.f;
	}

	void PhysicsControllerActor::SetHeight(float height)
	{
		if (!myController)
		{
			return;
		}

		GetCapsuleController()->setHeight(height);
		myController->resize(height);
	}

	void PhysicsControllerActor::SetRadius(float radius)
	{
		if (!myController)
		{
			return;
		}

		GetCapsuleController()->setRadius(radius);
	}

	void PhysicsControllerActor::Move(const glm::vec3& velocity)
	{
		myMovement += velocity;
	}

	void PhysicsControllerActor::Jump(float jumpPower)
	{
		myDownSpeed = -1.f * jumpPower;
	}

	void PhysicsControllerActor::SetPosition(const glm::vec3& position)
	{
		if (!myController)
		{
			return;
		}

		myController->setPosition(PhysXUtilities::ToPhysXVectorExtended(position));
	}

	void PhysicsControllerActor::SetFootPosition(const glm::vec3& position)
	{
		if (!myController)
		{
			return;
		}

		myController->setFootPosition(PhysXUtilities::ToPhysXVectorExtended(position));
	}

	void PhysicsControllerActor::SetAngularVelocity(const glm::vec3& velocity)
	{
		physx::PxRigidDynamic* actor = myController->getActor()->is<physx::PxRigidDynamic>();
		VT_CORE_ASSERT(actor, "Actor is null!");

		actor->setAngularVelocity(PhysXUtilities::ToPhysXVector(velocity));
	}

	const glm::vec3 PhysicsControllerActor::GetAngularVelocity() const
	{
		physx::PxRigidDynamic* actor = myController->getActor()->is<physx::PxRigidDynamic>();
		VT_CORE_ASSERT(actor, "Actor is null!");

		return PhysXUtilities::FromPhysXVector(actor->getAngularVelocity());
	}

	void PhysicsControllerActor::SetLinearVelocity(const glm::vec3& velocity)
	{
		physx::PxRigidDynamic* actor = myController->getActor()->is<physx::PxRigidDynamic>();
		VT_CORE_ASSERT(actor, "Actor is null!");

		actor->setLinearVelocity(PhysXUtilities::ToPhysXVector(velocity));

		myMovement.x = velocity.x;
		myDownSpeed = -velocity.y;
		myMovement.z = velocity.z;
	}

	const glm::vec3 PhysicsControllerActor::GetLinearVelocity() const
	{
		physx::PxRigidDynamic* actor = myController->getActor()->is<physx::PxRigidDynamic>();
		VT_CORE_ASSERT(actor, "Actor is null!");

		return PhysXUtilities::FromPhysXVector(actor->getLinearVelocity());
	}

	const bool PhysicsControllerActor::IsGrounded() const
	{
		return myCurrentCollisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
	}

	const bool PhysicsControllerActor::IsValid() const
	{
		return myController != nullptr;
	}

	const glm::vec3 PhysicsControllerActor::GetPosition() const
	{
		if (!myController)
		{
			return {};
		}

		return PhysXUtilities::FromPhysXVector(myController->getPosition());
	}

	const glm::vec3 PhysicsControllerActor::GetFootPosition() const
	{
		if (!myController)
		{
			return {};
		}

		return PhysXUtilities::FromPhysXVector(myController->getPosition());
	}

	void PhysicsControllerActor::SetSimulationData(uint32_t layerId)
	{
		const auto data = PhysXUtilities::CreateFilterDataFromLayer(layerId, CollisionDetectionType::Continuous);
		myFilterData = data;

		const uint32_t shapeCount = myController->getActor()->getNbShapes();
		std::vector<physx::PxShape*> shapes{ shapeCount };

		myController->getActor()->getShapes(shapes.data(), shapeCount);
		for (const auto& s : shapes)
		{
			s->setSimulationFilterData(myFilterData);
			s->setQueryFilterData(myFilterData);
		}

		myLayerId = layerId;
	}

	void PhysicsControllerActor::SetOffset(const glm::vec3& offset)
	{
		myOffset = offset;
	}

	void PhysicsControllerActor::SynchronizeTransform()
	{
		myEntity.SetPosition(GetPosition() - myOffset, false);
	}

	void PhysicsControllerActor::CreateController(physx::PxControllerManager* controllerManager)
	{
		myControllerData = myEntity.GetComponent<CharacterControllerComponent>();
		if (myEntity.HasComponent<CapsuleColliderComponent>())
		{
			auto& capsuleCollider = myEntity.GetComponent<CapsuleColliderComponent>();
			const float radiusScale = glm::max(myEntity.GetScale().x, myEntity.GetScale().z);

			physx::PxCapsuleControllerDesc desc{};
			desc.upDirection = { 0.f, 1.f, 0.f };
			desc.slopeLimit = std::max(0.f, glm::cos(glm::radians(myControllerData.slopeLimit)));
			desc.invisibleWallHeight = myControllerData.invisibleWallHeight;
			desc.maxJumpHeight = myControllerData.maxJumpHeight;
			desc.contactOffset = myControllerData.contactOffset;
			desc.stepOffset = myControllerData.stepOffset;
			desc.density = myControllerData.density;
			desc.userData = this;
			desc.radius = capsuleCollider.radius * radiusScale;
			desc.height = capsuleCollider.height * myEntity.GetScale().y;
			desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
			desc.position = PhysXUtilities::ToPhysXVectorExtended(myEntity.GetPosition());

			myMaterial = PhysXInternal::GetPhysXSDK().createMaterial(0.6f, 0.6f, 0.3f);
			desc.material = myMaterial;

			myController = controllerManager->createController(desc);
			myController->getActor()->userData = this;

#ifndef VT_DIST
			myController->getActor()->setName(myEntity.GetTag().c_str());
#endif

			myHasGravity = myControllerData.hasGravity;

			SetSimulationData(myControllerData.layer);
			SetOffset(myEntity.GetComponent<CapsuleColliderComponent>().offset);
		}
	}

	physx::PxCapsuleController* PhysicsControllerActor::GetCapsuleController()
	{
		return reinterpret_cast<physx::PxCapsuleController*>(myController);
	}
}
