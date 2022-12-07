#include "vtpch.h"
#include "ContactListener.h"

#include "Volt/Physics/PhysicsActor.h"
#include "Volt/Physics/PhysicsLayer.h"
#include "Volt/Scripting/ScriptBase.h"
#include "Volt/Components/Components.h"

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

		PhysicsActor* actorA = (PhysicsActor*)pairHeader.actors[0]->userData;
		PhysicsActor* actorB = (PhysicsActor*)pairHeader.actors[1]->userData;

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			Entity entityA = actorA->GetEntity();
			Entity entityB = actorB->GetEntity();

			if (entityA.IsNull() || entityB.IsNull())
			{
				return;
			}

			if (entityA.HasComponent<ScriptComponent>())
			{
				auto& scriptComp = entityA.GetComponent<ScriptComponent>();
				for (const auto& script : scriptComp.scripts)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
						{
							Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(entityA.GetId(), script);
							if (scriptInstance)
							{
								scriptInstance->OnCollisionEnter(entityB);
							}
						});
				}
			}

			if (entityB.HasComponent<ScriptComponent>())
			{
				auto& scriptComp = entityB.GetComponent<ScriptComponent>();
				for (const auto& script : scriptComp.scripts)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
						{
							Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(entityB.GetId(), script);
							if (scriptInstance)
							{
								scriptInstance->OnCollisionEnter(entityA);
							}
						});
				}
			}
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			Entity entityA = actorA->GetEntity();
			Entity entityB = actorB->GetEntity();

			if (entityA.IsNull() || entityB.IsNull())
			{
				return;
			}

			if (entityA.HasComponent<ScriptComponent>())
			{
				auto& scriptComp = entityA.GetComponent<ScriptComponent>();
				for (const auto& script : scriptComp.scripts)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
						{
							Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(entityA.GetId(), script);
							if (scriptInstance)
							{
								scriptInstance->OnCollisionExit(entityB);
							}
						});
				}
			}

			if (entityB.HasComponent<ScriptComponent>())
			{
				auto& scriptComp = entityB.GetComponent<ScriptComponent>();
				for (const auto& script : scriptComp.scripts)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
						{
							Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(entityB.GetId(), script);
							if (scriptInstance)
							{
								scriptInstance->OnCollisionExit(entityA);
							}
						});
				}
			}
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

			PhysicsActor* triggerActor = (PhysicsActor*)pairs[i].triggerActor->userData;
			PhysicsActor* otherActor = (PhysicsActor*)pairs[i].otherActor->userData;

			if (!triggerActor || !otherActor)
			{
				continue;
			}

			if (!PhysicsLayerManager::ShouldCollide(triggerActor->GetRigidbodyData().layerId, otherActor->GetRigidbodyData().layerId))
			{
				continue;
			}

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				Entity triggerEntity = triggerActor->GetEntity();
				Entity otherEntity = otherActor->GetEntity();

				if (triggerEntity.IsNull() || otherEntity.IsNull())
				{
					return;
				}

				if (triggerEntity.HasComponent<ScriptComponent>())
				{
					auto& scriptComp = triggerEntity.GetComponent<ScriptComponent>();
					for (const auto& script : scriptComp.scripts)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]()
							{
								Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(triggerEntity.GetId(), script);
								if (scriptInstance)
								{
									scriptInstance->OnTriggerEnter(otherEntity, false);
								}
							});

					}
				}

				if (otherEntity.HasComponent<ScriptComponent>())
				{
					auto& scriptComp = otherEntity.GetComponent<ScriptComponent>();
					for (const auto& script : scriptComp.scripts)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]()
							{
								Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(otherEntity.GetId(), script);
								if (scriptInstance)
								{
									scriptInstance->OnTriggerEnter(triggerEntity, false);
								}
							});
					}
				}
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				Entity triggerEntity = triggerActor->GetEntity();
				Entity otherEntity = otherActor->GetEntity();

				if (triggerEntity.IsNull() || otherEntity.IsNull())
				{
					return;
				}

				if (triggerEntity.HasComponent<ScriptComponent>())
				{
					auto& scriptComp = triggerEntity.GetComponent<ScriptComponent>();
					for (const auto& script : scriptComp.scripts)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]
							{
								Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(triggerEntity.GetId(), script);
								if (scriptInstance)
								{
									scriptInstance->OnTriggerExit(otherEntity, false);
								}
							});

					}
				}

				if (otherEntity.HasComponent<ScriptComponent>())
				{
					auto& scriptComp = otherEntity.GetComponent<ScriptComponent>();
					for (const auto& script : scriptComp.scripts)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]
							{
								Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(otherEntity.GetId(), script);
								if (scriptInstance)
								{
									scriptInstance->OnTriggerExit(triggerEntity, false);
								}
							});
					}
				}
			}
		}
	}

	void ContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
	{
		PX_UNUSED(bodyBuffer);
		PX_UNUSED(poseBuffer);
		PX_UNUSED(count);
	}
	
	void ContactListener::RunEvents()
	{
		for (const auto& e : myFrameEvents)
		{
			e();
		}

		myFrameEvents.clear();
	}
}