#include "vtpch.h"
#include "Volt/Physics/ContactListener.h"

#include "Volt/Physics/PhysicsActor.h"
#include "Volt/Physics/PhysicsLayer.h"
#include "Volt/Physics/PhysicsEvents.h"

#include <EventSystem/EventSystem.h>

namespace Volt
{
	void ContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		PX_UNUSED(constraints);
		PX_UNUSED(count);
	}

	void ContactListener::onWake(physx::PxActor** actors, physx::PxU32 count)
	{
		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void ContactListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void ContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32)
	{
		auto removedActorA = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
		auto removedActorB = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

		if (removedActorA || removedActorB)
		{
			return;
		}

		PhysicsActorBase* actorA = (PhysicsActorBase*)pairHeader.actors[0]->userData;
		PhysicsActorBase* actorB = (PhysicsActorBase*)pairHeader.actors[1]->userData;

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			Entity entityA = actorA->GetEntity();
			Entity entityB = actorB->GetEntity();

			if (!entityA.IsValid() || !entityB.IsValid())
			{
				return;
			}

			EntityHelper helperA = entityA.GetScene()->GetEntityHelperFromEntityID(entityA.GetID());
			EntityHelper helperB = entityB.GetScene()->GetEntityHelperFromEntityID(entityB.GetID());

			OnCollisionEnterEvent enterEvent(helperA, helperB);
			EventSystem::DispatchEvent<OnCollisionEnterEvent>(enterEvent);
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			Entity entityA = actorA->GetEntity();
			Entity entityB = actorB->GetEntity();

			if (!entityA.IsValid() || !entityB.IsValid())
			{
				return;
			}

			EntityHelper helperA = entityA.GetScene()->GetEntityHelperFromEntityID(entityA.GetID());
			EntityHelper helperB = entityB.GetScene()->GetEntityHelperFromEntityID(entityB.GetID());

			OnCollisionExitEvent exitEvent(helperA, helperB);
			EventSystem::DispatchEvent<OnCollisionExitEvent>(exitEvent);
		}
	}

	void ContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
			{
				continue;
			}

			PhysicsActorBase* triggerActor = (PhysicsActorBase*)pairs[i].triggerActor->userData;
			PhysicsActorBase* otherActor = (PhysicsActorBase*)pairs[i].otherActor->userData;

			if (!triggerActor || !otherActor)
			{
				continue;
			}

			if (!PhysicsLayerManager::ShouldCollide(triggerActor->GetLayerId(), otherActor->GetLayerId()))
			{
				continue;
			}

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				Entity triggerEntity = triggerActor->GetEntity();
				Entity otherEntity = otherActor->GetEntity();

				if (!triggerEntity.IsValid() || !otherEntity.IsValid())
				{
					return;
				}

				EntityHelper triggerHelper = triggerEntity.GetScene()->GetEntityHelperFromEntityID(triggerEntity.GetID());
				EntityHelper otherHelper = otherEntity.GetScene()->GetEntityHelperFromEntityID(otherEntity.GetID());

				OnTriggerEnterEvent enterEvent(triggerHelper, otherHelper);
				EventSystem::DispatchEvent<OnTriggerEnterEvent>(enterEvent);
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				Entity triggerEntity = triggerActor->GetEntity();
				Entity otherEntity = otherActor->GetEntity();

				if (!triggerEntity.IsValid() || !otherEntity.IsValid())
				{
					return;
				}

				EntityHelper triggerHelper = triggerEntity.GetScene()->GetEntityHelperFromEntityID(triggerEntity.GetID());
				EntityHelper otherHelper = otherEntity.GetScene()->GetEntityHelperFromEntityID(otherEntity.GetID());

				OnTriggerExitEvent exitEvent(triggerHelper, otherHelper);
				EventSystem::DispatchEvent<OnTriggerExitEvent>(exitEvent);
			}
		}
	}

	void ContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
	{
		PX_UNUSED(bodyBuffer);
		PX_UNUSED(poseBuffer);
		PX_UNUSED(count);
	}

	physx::PxQueryHitType::Enum CharacterControllerContactListener::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor*, physx::PxHitFlags&)
	{
		if ((filterData.word0 & shape->getQueryFilterData().word1) || (filterData.word1 & shape->getQueryFilterData().word0))
		{
			if (shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE))
			{
				return physx::PxQueryHitType::eTOUCH;
			}
			else
			{
				return physx::PxQueryHitType::eBLOCK;
			}
		}
		return physx::PxQueryHitType::eNONE;
	}
}
