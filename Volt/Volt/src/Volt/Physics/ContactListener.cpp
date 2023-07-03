#include "vtpch.h"
#include "ContactListener.h"

#include "Volt/Physics/PhysicsActor.h"
#include "Volt/Physics/PhysicsLayer.h"
#include "Volt/Components/Components.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"

#include <GraphKey/Nodes/PhysicsNodes.h>

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

			if (entityA.IsNull() || entityB.IsNull())
			{
				return;
			}

			if (entityA.HasComponent<MonoScriptComponent>() && entityA.IsVisible())
			{
				auto& scriptComp = entityA.GetComponent<MonoScriptComponent>();
				for (const auto& script : scriptComp.scriptIds)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
					{
						Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
						if (scriptInstance)
						{
							scriptInstance->InvokeOnCollisionEnter(entityB);
						}
					});
				}
			}

			if (entityB.HasComponent<MonoScriptComponent>() && entityB.IsVisible())
			{
				auto& scriptComp = entityB.GetComponent<MonoScriptComponent>();
				for (const auto& script : scriptComp.scriptIds)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
					{
						Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
						if (scriptInstance)
						{
							scriptInstance->InvokeOnCollisionEnter(entityA);
						}
					});
				}
			}

			if (entityA.HasComponent<VisualScriptingComponent>() && entityA.IsVisible())
			{
				auto& scriptComp = entityA.GetComponent<VisualScriptingComponent>();
				if (scriptComp.graph)
				{
					myFrameEvents.emplace_back([scriptComp, entityA, entityB]()
					{
						GraphKey::OnCollisionEnterEvent e{ entityB };
						scriptComp.graph->OnEvent(e);
					});
				}
			}

			if (entityB.HasComponent<VisualScriptingComponent>() && entityB.IsVisible())
			{
				auto& scriptComp = entityB.GetComponent<VisualScriptingComponent>();
				if (scriptComp.graph)
				{
					myFrameEvents.emplace_back([scriptComp, entityA, entityB]()
					{
						GraphKey::OnCollisionEnterEvent e{ entityA };
						scriptComp.graph->OnEvent(e);
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

			if (entityA.HasComponent<MonoScriptComponent>() && entityA.IsVisible())
			{
				auto& scriptComp = entityA.GetComponent<MonoScriptComponent>();
				for (const auto& script : scriptComp.scriptIds)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
					{
						Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
						if (scriptInstance)
						{
							scriptInstance->InvokeOnCollisionExit(entityB);
						}
					});
				}
			}

			if (entityB.HasComponent<MonoScriptComponent>() && entityB.IsVisible())
			{
				auto& scriptComp = entityB.GetComponent<MonoScriptComponent>();
				for (const auto& script : scriptComp.scriptIds)
				{
					myFrameEvents.emplace_back([script, entityA, entityB]()
					{
						Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
						if (scriptInstance)
						{
							scriptInstance->InvokeOnCollisionExit(entityA);
						}
					});
				}
			}

			if (entityA.HasComponent<VisualScriptingComponent>() && entityA.IsVisible())
			{
				auto& scriptComp = entityA.GetComponent<VisualScriptingComponent>();
				if (scriptComp.graph)
				{
					myFrameEvents.emplace_back([scriptComp, entityA, entityB]()
					{
						GraphKey::OnCollisionExitEvent e{ entityB };
						scriptComp.graph->OnEvent(e);
					});
				}
			}

			if (entityB.HasComponent<VisualScriptingComponent>() && entityB.IsVisible())
			{
				auto& scriptComp = entityB.GetComponent<VisualScriptingComponent>();
				if (scriptComp.graph)
				{
					myFrameEvents.emplace_back([scriptComp, entityA, entityB]()
					{
						GraphKey::OnCollisionExitEvent e{ entityA };
						scriptComp.graph->OnEvent(e);
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

				if (triggerEntity.IsNull() || otherEntity.IsNull())
				{
					return;
				}

				if (triggerEntity.HasComponent<MonoScriptComponent>() && triggerEntity.IsVisible())
				{
					auto& scriptComp = triggerEntity.GetComponent<MonoScriptComponent>();
					for (const auto& script : scriptComp.scriptIds)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]()
						{
							Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
							if (scriptInstance)
							{
								scriptInstance->InvokeOnTriggerEnter(otherEntity);
							}
						});
					}
				}

				if (otherEntity.HasComponent<MonoScriptComponent>() && otherEntity.IsVisible())
				{
					auto& scriptComp = otherEntity.GetComponent<MonoScriptComponent>();
					for (const auto& script : scriptComp.scriptIds)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]()
						{
							Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
							if (scriptInstance)
							{
								scriptInstance->InvokeOnTriggerEnter(triggerEntity);
							}
						});
					}
				}

				if (triggerEntity.HasComponent<VisualScriptingComponent>() && triggerEntity.IsVisible())
				{
					auto& scriptComp = triggerEntity.GetComponent<VisualScriptingComponent>();
					if (scriptComp.graph)
					{
						myFrameEvents.emplace_back([scriptComp, triggerEntity, otherEntity]()
						{
							GraphKey::OnTriggerEnterEvent e{ otherEntity };
							scriptComp.graph->OnEvent(e);
						});
					}
				}

				if (otherEntity.HasComponent<VisualScriptingComponent>() && otherEntity.IsVisible())
				{
					auto& scriptComp = otherEntity.GetComponent<VisualScriptingComponent>();
					if (scriptComp.graph)
					{
						myFrameEvents.emplace_back([scriptComp, triggerEntity, otherEntity]()
						{
							GraphKey::OnTriggerEnterEvent e{ triggerEntity };
							scriptComp.graph->OnEvent(e);
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

				if (triggerEntity.HasComponent<MonoScriptComponent>() && triggerEntity.IsVisible())
				{
					auto& scriptComp = triggerEntity.GetComponent<MonoScriptComponent>();
					for (const auto& script : scriptComp.scriptIds)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]()
						{
							Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
							if (scriptInstance)
							{
								scriptInstance->InvokeOnTriggerExit(otherEntity);
							}
						});

					}
				}

				if (otherEntity.HasComponent<MonoScriptComponent>() && otherEntity.IsVisible())
				{
					auto& scriptComp = otherEntity.GetComponent<MonoScriptComponent>();
					for (const auto& script : scriptComp.scriptIds)
					{
						myFrameEvents.emplace_back([script, triggerEntity, otherEntity]()
						{
							Ref<MonoScriptInstance> scriptInstance = MonoScriptEngine::GetInstanceFromId(script);
							if (scriptInstance)
							{
								scriptInstance->InvokeOnTriggerExit(triggerEntity);
							}
						});
					}
				}

				if (triggerEntity.HasComponent<VisualScriptingComponent>() && triggerEntity.IsVisible())
				{
					auto& scriptComp = triggerEntity.GetComponent<VisualScriptingComponent>();
					if (scriptComp.graph)
					{
						myFrameEvents.emplace_back([scriptComp, triggerEntity, otherEntity]()
						{
							GraphKey::OnTriggerExitEvent e{ otherEntity };
							scriptComp.graph->OnEvent(e);
						});
					}
				}

				if (otherEntity.HasComponent<VisualScriptingComponent>() && otherEntity.IsVisible())
				{
					auto& scriptComp = otherEntity.GetComponent<VisualScriptingComponent>();
					if (scriptComp.graph)
					{
						myFrameEvents.emplace_back([scriptComp, triggerEntity, otherEntity]()
						{
							GraphKey::OnTriggerExitEvent e{ triggerEntity };
							scriptComp.graph->OnEvent(e);
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
