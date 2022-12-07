#include "vtpch.h"
#include "PhysicsScene.h"

#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsShapes.h"
#include "Volt/Physics/PhysicsLayer.h"

#include "Volt/Scripting/ScriptBase.h"
#include "Volt/Components/Components.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Log/Log.h"

#include <PhysX/PxActor.h>

namespace Volt
{
	PhysicsScene::PhysicsScene(const PhysicsSettings& physicsSettings, Scene* entityScene)
		: mySubStepSize(physicsSettings.fixedTimestep), myEntityScene(entityScene)
	{
		physx::PxSceneDesc sceneDesc{ PhysXInternal::GetPhysXSDK().getTolerancesScale() };
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDesc.gravity = PhysXUtilities::ToPhysXVector(physicsSettings.gravity);
		sceneDesc.broadPhaseType = PhysXInternal::VoltToPhysXBroadphase(physicsSettings.broadphaseAlgorithm);
		sceneDesc.cpuDispatcher = PhysXInternal::GetCPUDispatcher();
		sceneDesc.filterShader = (physx::PxSimulationFilterShader)PhysXInternal::FilterShader;
		sceneDesc.simulationEventCallback = &myContactListener;
		sceneDesc.frictionType = PhysXInternal::VoltToPhysXFrictionType(physicsSettings.frictionModel);

		VT_CORE_ASSERT(sceneDesc.isValid(), "Physics scene not valid!");

		myPhysXScene = PhysXInternal::GetPhysXSDK().createScene(sceneDesc);
		VT_CORE_ASSERT(myPhysXScene, "PhysX scene not valid!");

#ifndef VT_DIST
		myPhysXScene->getScenePvdClient()->setScenePvdFlags(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS | physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES | physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS);
#endif 

		CreateRegions();
	}

	PhysicsScene::~PhysicsScene()
	{}

	void PhysicsScene::Simulate(float timeStep)
	{
		VT_PROFILE_FUNCTION();

		if (myEntityScene->IsPlaying())
		{
			myUpdateAccumulator += timeStep;
			if (myUpdateAccumulator >= mySubStepSize)
			{
				myEntityScene->GetRegistry().ForEach<ScriptComponent>([this](Wire::EntityId id, ScriptComponent& scriptComp)
					{
						for (const auto& script : scriptComp.scripts)
						{
							Ref<ScriptBase> scriptInstance = ScriptEngine::GetScript(id, script);
							if (scriptInstance)
							{
								scriptInstance->OnFixedUpdate(mySubStepSize);
							}
						}
					});

				myUpdateAccumulator = 0.f;
			}
		}

		bool advanced = Advance(timeStep);
		if (advanced)
		{
			uint32_t activeActorCount = 0;
			physx::PxActor** activeActors = myPhysXScene->getActiveActors(activeActorCount);

			for (uint32_t i = 0; i < activeActorCount; i++)
			{
				PhysicsActor* actor = (PhysicsActor*)activeActors[i]->userData;
				actor->SynchronizeTransform();
			}

		}

		for (const auto& f : myFunctionQueue)
		{
			f();
		}

		myFunctionQueue.clear();

		myContactListener.RunEvents();

	}

	Ref<PhysicsActor> PhysicsScene::GetActor(Entity entity)
	{
		for (const auto& actor : myPhysicsActors)
		{
			if (actor->GetEntity().GetId() == entity.GetId())
			{
				return actor;
			}
		}

		return nullptr;
	}

	const Ref<PhysicsActor> PhysicsScene::GetActor(Entity entity) const
	{
		for (const auto& actor : myPhysicsActors)
		{
			if (actor->GetEntity().GetId() == entity.GetId())
			{
				return actor;
			}
		}

		return nullptr;
	}

	Ref<PhysicsActor> PhysicsScene::CreateActor(Entity entity)
	{
		Ref<PhysicsActor> actor = CreateRef<PhysicsActor>(entity);
		myPhysicsActors.emplace_back(actor);

		auto func = [&, addedEntity = entity]()
		{
			auto addedActor = GetActor(addedEntity);
			myPhysXScene->addActor(*addedActor->myRigidActor);
		};

		if (myInSimulation)
		{
			myFunctionQueue.emplace_back(func);
		}
		else
		{
			func();
		}

		return actor;
	}

	void PhysicsScene::RemoveActor(Ref<PhysicsActor> actor)
	{
		actor->myToBeRemoved = true;

		auto func = [&, addedActor = actor]()
		{
			for (auto& collider : addedActor->myColliders)
			{
				collider->DetachFromActor(addedActor->myRigidActor);
				collider->Release();
			}

			myPhysXScene->removeActor(*addedActor->myRigidActor);
			addedActor->myRigidActor->release();
			addedActor->myRigidActor = nullptr;
		};

		if (myInSimulation)
		{
			myFunctionQueue.emplace_back(func);
		}
		else
		{
			func();
		}

		auto it = std::find_if(myPhysicsActors.begin(), myPhysicsActors.end(), [actor](const Ref<PhysicsActor>& a)
			{
				return actor->GetEntity().GetId() == a->GetEntity().GetId();
			});

		if (it != myPhysicsActors.end())
		{
			myPhysicsActors.erase(it);
		}
	}

	bool PhysicsScene::Raycast(const gem::vec3& origin, const gem::vec3& direction, float maxDistance, RaycastHit* outHit)
	{
		physx::PxRaycastBuffer hitInfo{};
		bool result = myPhysXScene->raycast(PhysXUtilities::ToPhysXVector(origin), PhysXUtilities::ToPhysXVector(gem::normalize(direction)), maxDistance, hitInfo);

		if (result)
		{
			PhysicsActor* actor = (PhysicsActor*)hitInfo.block.actor->userData;
			outHit->hitEntity = actor->GetEntity().GetId();
			outHit->position = PhysXUtilities::FromPhysXVector(hitInfo.block.position);
			outHit->normal = PhysXUtilities::FromPhysXVector(hitInfo.block.normal);
			outHit->distance = hitInfo.block.distance;
		}

		return result;
	}

	bool PhysicsScene::Raycast(const gem::vec3& origin, const gem::vec3& direction, float maxDistance, RaycastHit* outHit, uint32_t layerMask)
	{
		const auto& layer = PhysicsLayerManager::GetLayer(layerMask);

		physx::PxFilterData data{};
		data.word0 = layer.bitValue;

		physx::PxQueryFilterData qFilterData;
		qFilterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;
		qFilterData.data = data;

		physx::PxRaycastBuffer hitInfo{};
		bool result = myPhysXScene->raycast(PhysXUtilities::ToPhysXVector(origin), PhysXUtilities::ToPhysXVector(gem::normalize(direction)), maxDistance, hitInfo, physx::PxHitFlag::eDEFAULT, qFilterData);

		if (result)
		{
			PhysicsActor* actor = (PhysicsActor*)hitInfo.block.actor->userData;
			outHit->hitEntity = actor->GetEntity().GetId();
			outHit->position = PhysXUtilities::FromPhysXVector(hitInfo.block.position);
			outHit->normal = PhysXUtilities::FromPhysXVector(hitInfo.block.normal);
			outHit->distance = hitInfo.block.distance;
		}

		return result;
	}

	bool PhysicsScene::OverlapBox(const gem::vec3& origin, const gem::vec3& halfSize, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxBoxGeometry(halfSize.x, halfSize.y, halfSize.z), buffer, count);
	}

	bool PhysicsScene::OverlapCapsule(const gem::vec3& origin, float radius, float halfHeight, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxCapsuleGeometry(radius, halfHeight), buffer, count);
	}

	bool PhysicsScene::OverlapSphere(const gem::vec3& origin, float radius, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxSphereGeometry(radius), buffer, count);
	}

	void PhysicsScene::CreateRegions()
	{
		const PhysicsSettings& settings = Physics::GetSettings();

		if (settings.broadphaseAlgorithm == BroadphaseType::AutomaticBoxPrune)
		{
			return;
		}

		physx::PxBounds3* regionBounds = new physx::PxBounds3[settings.worldBoundsSubDivisions * settings.worldBoundsSubDivisions];
		physx::PxBounds3 globalBounds(PhysXUtilities::ToPhysXVector(settings.worldBoundsMin), PhysXUtilities::ToPhysXVector(settings.worldBoundsMax));
		uint32_t regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, settings.worldBoundsSubDivisions);

		for (uint32_t i = 0; i < regionCount; i++)
		{
			physx::PxBroadPhaseRegion region;
			region.mBounds = regionBounds[i];
			myPhysXScene->addBroadPhaseRegion(region);
		}
	}

	bool PhysicsScene::Advance(float timeStep)
	{
		SubstepStrategy(timeStep);

		for (uint32_t i = 0; i < myNumSubSteps; i++)
		{
			myInSimulation = true;
			myPhysXScene->simulate(mySubStepSize);
			myPhysXScene->fetchResults(true);
		}

		myInSimulation = false;

		return myNumSubSteps != 0;
	}

	void PhysicsScene::SubstepStrategy(float timeStep)
	{
		if (myAccumulator > mySubStepSize)
		{
			myAccumulator = 0.f;
		}

		myAccumulator += timeStep;
		if (myAccumulator < mySubStepSize)
		{
			myNumSubSteps = 0;
			return;
		}

		myNumSubSteps = gem::min(static_cast<uint32_t>(myAccumulator / mySubStepSize), myMaxSubSteps);
		myAccumulator -= (float)myNumSubSteps * mySubStepSize;
	}

	void PhysicsScene::Destroy()
	{
		VT_CORE_ASSERT(myPhysXScene, "Trying to destroy invalid physics scene!");

		myInSimulation = true;
		for (int32_t i = (int32_t)myPhysicsActors.size() - 1; i >= 0; --i)
		{
			RemoveActor(myPhysicsActors[i]);
		}
		myPhysicsActors.clear();

		for (const auto& f : myFunctionQueue)
		{
			f();
		}

		myFunctionQueue.clear();

		myPhysXScene->release();
		myPhysXScene = nullptr;
	}

	bool PhysicsScene::OverlapGeometry(const gem::vec3& origin, const physx::PxGeometry& geometry, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count)
	{
		physx::PxOverlapBuffer buf(buffer.data(), MAX_OVERLAP_COLLIDERS);
		physx::PxTransform pose = PhysXUtilities::ToPhysXTransform(gem::translate(gem::mat4(1.0f), origin));

		bool result = myPhysXScene->overlap(geometry, pose, buf);
		if (result)
		{
			memcpy(buffer.data(), buf.touches, buf.nbTouches * sizeof(physx::PxOverlapHit));
			count = buf.nbTouches;
		}

		return result;
	}
}