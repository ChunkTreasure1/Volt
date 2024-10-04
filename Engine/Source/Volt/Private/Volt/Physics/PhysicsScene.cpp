#include "vtpch.h"
#include "Volt/Physics/PhysicsScene.h"

#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsShapes.h"
#include "Volt/Physics/PhysicsLayer.h"

#include "Volt/Core/Application.h"

#include "Volt/Components/CoreComponents.h"

#include <PhysX/PxActor.h>

namespace Volt
{
	PhysicsScene::PhysicsScene(const PhysicsSettings& physicsSettings, Scene* entityScene)
		: m_subStepSize(physicsSettings.fixedTimestep), m_entityScene(entityScene)
	{
		physx::PxSceneDesc sceneDesc{ PhysXInternal::GetPhysXSDK().getTolerancesScale() };
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDesc.gravity = PhysXUtilities::ToPhysXVector(physicsSettings.gravity);
		sceneDesc.broadPhaseType = PhysXInternal::VoltToPhysXBroadphase(physicsSettings.broadphaseAlgorithm);
		sceneDesc.cpuDispatcher = PhysXInternal::GetCPUDispatcher();
		sceneDesc.filterShader = (physx::PxSimulationFilterShader)PhysXInternal::FilterShader;
		sceneDesc.simulationEventCallback = &m_contactListener;
		sceneDesc.frictionType = PhysXInternal::VoltToPhysXFrictionType(physicsSettings.frictionModel);

		VT_ASSERT_MSG(sceneDesc.isValid(), "Physics scene not valid!");

		m_physXScene = PhysXInternal::GetPhysXSDK().createScene(sceneDesc);
		VT_ASSERT_MSG(m_physXScene, "PhysX scene not valid!");

		m_controllerManager = PxCreateControllerManager(*m_physXScene);
		m_controllerManager->setTessellation(true, 100.f);

#ifndef VT_DIST
		if (!Application::Get().IsRuntime())
		{
			m_physXScene->getScenePvdClient()->setScenePvdFlags(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS | physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES | physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS);
		}
#endif

		CreateRegions();
	}

	PhysicsScene::~PhysicsScene()
	{
	}

	void PhysicsScene::Simulate(float timeStep)
	{
		VT_PROFILE_FUNCTION();

		bool advanced = Advance(timeStep);
		if (advanced)
		{
			uint32_t activeActorCount = 0;
			physx::PxActor** activeActors = m_physXScene->getActiveActors(activeActorCount);

			for (uint32_t i = 0; i < activeActorCount; i++)
			{
				PhysicsActor* actor = (PhysicsActor*)activeActors[i]->userData;
				if (actor)
				{
					actor->SynchronizeTransform();
				}
			}

			m_entityScene->FixedUpdate(timeStep);
		}

		if (m_entityScene->IsPlaying())
		{
			if (advanced)
			{
				// #TODO_Ivar: Implement with new system.
			}
		}

		{
			VT_PROFILE_SCOPE("Update actors");
			for (const auto& actor : m_controllerActors)
			{
				actor->Update(timeStep);
			}
		}

		{
			VT_PROFILE_SCOPE("Synchronize Transform");
			for (const auto& actor : m_controllerActors)
			{
				actor->SynchronizeTransform();
			}
		}

		for (const auto& f : m_functionQueue)
		{
			f();
		}

		m_functionQueue.clear();
	}

	Ref<PhysicsActor> PhysicsScene::GetActor(Entity entity)
	{
		if (m_physicsActorFromEntityIDMap.contains(entity.GetHandle()))
		{
			return m_physicsActorFromEntityIDMap.at(entity.GetHandle());
		}

		return nullptr;
	}

	const Ref<PhysicsActor> PhysicsScene::GetActor(Entity entity) const
	{
		if (m_physicsActorFromEntityIDMap.contains(entity.GetHandle()))
		{
			return m_physicsActorFromEntityIDMap.at(entity.GetHandle());
		}

		return nullptr;
	}

	Ref<PhysicsControllerActor> PhysicsScene::GetControllerActor(Entity entity)
	{
		if (m_physicsControllerActorFromEntityIDMap.contains(entity.GetHandle()))
		{
			return m_physicsControllerActorFromEntityIDMap.at(entity.GetHandle());
		}

		return nullptr;
	}

	const Ref<PhysicsControllerActor> PhysicsScene::GetControllerActor(Entity entity) const
	{
		if (m_physicsControllerActorFromEntityIDMap.contains(entity.GetHandle()))
		{
			return m_physicsControllerActorFromEntityIDMap.at(entity.GetHandle());
		}

		return nullptr;
	}

	Ref<PhysicsActor> PhysicsScene::CreateActor(Entity entity)
	{
		Ref<PhysicsActor> actor = CreateRef<PhysicsActor>(entity);
		m_physicsActors.emplace_back(actor);

		m_physicsActorFromEntityIDMap[entity.GetHandle()] = actor;

		auto func = [&, addedEntity = entity]()
		{
			auto addedActor = GetActor(addedEntity);
			m_physXScene->addActor(*addedActor->m_rigidActor);
			addedActor->SetPosition(addedEntity.GetPosition());
		};

		if (m_inSimulation)
		{
			m_functionQueue.emplace_back(func);
		}
		else
		{
			func();
		}

		return actor;
	}

	void PhysicsScene::RemoveActor(Ref<PhysicsActor> actor)
	{
		actor->m_toBeRemoved = true;

		auto func = [&, addedActor = actor]()
		{
			for (auto& collider : addedActor->m_colliders)
			{
				collider->DetachFromActor(addedActor->m_rigidActor);
				collider->Release();
			}

			m_physXScene->removeActor(*addedActor->m_rigidActor);
			addedActor->m_rigidActor->release();
			addedActor->m_rigidActor = nullptr;
		};

		if (m_inSimulation)
		{
			m_functionQueue.emplace_back(func);
		}
		else
		{
			func();
		}

		auto actorEntityHandle = actor->GetEntity().GetHandle();

		auto it = std::find_if(m_physicsActors.begin(), m_physicsActors.end(), [actorEntityHandle](const Ref<PhysicsActor>& a)
		{
			return a->GetEntity().GetHandle() == actorEntityHandle;
		});

		if (it != m_physicsActors.end())
		{
			m_physicsActors.erase(it);
		}

		if (m_physicsActorFromEntityIDMap.contains(actorEntityHandle))
		{
			m_physicsActorFromEntityIDMap.erase(actorEntityHandle);
		}
	}

	Ref<PhysicsControllerActor> PhysicsScene::CreateControllerActor(Entity entity)
	{
		Ref<PhysicsControllerActor> actor = CreateRef<PhysicsControllerActor>(entity);
		m_controllerActors.emplace_back(actor);
		m_physicsControllerActorFromEntityIDMap[entity.GetHandle()] = actor;

		auto func = [&, addedEntity = entity]()
		{
			auto controllerActor = GetControllerActor(addedEntity);
			controllerActor->Create(m_controllerManager);
			controllerActor->SetPosition(entity.GetPosition());
		};

		if (m_inSimulation)
		{
			m_functionQueue.emplace_back(func);
		}
		else
		{
			func();
		}

		return actor;
	}

	void PhysicsScene::RemoveControllerActor(Ref<PhysicsControllerActor> controllerActor)
	{
		auto actorEntityHandle = controllerActor->GetEntity().GetHandle();

		auto it = std::find_if(m_controllerActors.begin(), m_controllerActors.end(), [actorEntityHandle](const auto& lhs)
		{
			return lhs->GetEntity().GetHandle() == actorEntityHandle;
		});

		if (it != m_controllerActors.end())
		{
			m_controllerActors.erase(it);
		}

		if (m_physicsControllerActorFromEntityIDMap.contains(actorEntityHandle))
		{
			m_physicsControllerActorFromEntityIDMap.erase(actorEntityHandle);
		}
	}

	bool PhysicsScene::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit)
	{
		physx::PxRaycastBuffer hitInfo{};

		bool result = m_physXScene->raycast(PhysXUtilities::ToPhysXVector(origin), PhysXUtilities::ToPhysXVector(glm::normalize(direction)), maxDistance, hitInfo);

		if (result)
		{
			PhysicsActorBase* actor = (PhysicsActorBase*)hitInfo.block.actor->userData;
			outHit->hitEntity = actor->GetEntity().GetID();
			outHit->position = PhysXUtilities::FromPhysXVector(hitInfo.block.position);
			outHit->normal = PhysXUtilities::FromPhysXVector(hitInfo.block.normal);
			outHit->distance = hitInfo.block.distance;
		}

		return result;
	}

	bool PhysicsScene::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit, uint32_t layerMask)
	{
		physx::PxFilterData data{};
		data.word0 = layerMask;

		physx::PxQueryFilterData qFilterData;
		qFilterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;
		qFilterData.data = data;

		physx::PxRaycastBuffer hitInfo{};
		bool result = m_physXScene->raycast(PhysXUtilities::ToPhysXVector(origin), PhysXUtilities::ToPhysXVector(glm::normalize(direction)), maxDistance, hitInfo, physx::PxHitFlag::eDEFAULT, qFilterData);

		if (result)
		{
			PhysicsActorBase* actor = (PhysicsActorBase*)hitInfo.block.actor->userData;
			outHit->hitEntity = actor->GetEntity().GetID();
			outHit->position = PhysXUtilities::FromPhysXVector(hitInfo.block.position);
			outHit->normal = PhysXUtilities::FromPhysXVector(hitInfo.block.normal);
			outHit->distance = hitInfo.block.distance;
		}

		return result;
	}

	bool PhysicsScene::Linecast(const glm::vec3& origin, const glm::vec3& destination, RaycastHit* outHit)
	{
		physx::PxRaycastBuffer hitInfo{};

		glm::vec3 direction = destination - origin;
		float distance = glm::distance(destination, origin);

		bool result = m_physXScene->raycast(PhysXUtilities::ToPhysXVector(origin), PhysXUtilities::ToPhysXVector(glm::normalize(direction)), distance, hitInfo);

		if (result)
		{
			PhysicsActorBase* actor = (PhysicsActorBase*)hitInfo.block.actor->userData;
			outHit->hitEntity = actor->GetEntity().GetID();
			outHit->position = PhysXUtilities::FromPhysXVector(hitInfo.block.position);
			outHit->normal = PhysXUtilities::FromPhysXVector(hitInfo.block.normal);
			outHit->distance = hitInfo.block.distance;
		}

		return result;
	}

	bool PhysicsScene::Linecast(const glm::vec3& origin, const glm::vec3& destination, RaycastHit* outHit, uint32_t layerMask)
	{
		const auto& layer = PhysicsLayerManager::GetLayer(layerMask);

		physx::PxFilterData data{};
		data.word0 = layer.bitValue;

		physx::PxQueryFilterData qFilterData;
		qFilterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;
		qFilterData.data = data;

		physx::PxRaycastBuffer hitInfo{};
		glm::vec3 direction = destination - origin;
		float distance = glm::distance(destination, origin);
		bool result = m_physXScene->raycast(PhysXUtilities::ToPhysXVector(origin), PhysXUtilities::ToPhysXVector(glm::normalize(direction)), distance, hitInfo, physx::PxHitFlag::eDEFAULT, qFilterData);

		if (result)
		{
			PhysicsActorBase* actor = (PhysicsActorBase*)hitInfo.block.actor->userData;
			outHit->hitEntity = actor->GetEntity().GetID();
			outHit->position = PhysXUtilities::FromPhysXVector(hitInfo.block.position);
			outHit->normal = PhysXUtilities::FromPhysXVector(hitInfo.block.normal);
			outHit->distance = hitInfo.block.distance;
		}

		return result;
	}

	bool PhysicsScene::OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, Vector<Entity>& hitList, uint32_t layerMask)
	{
		const auto& layer = PhysicsLayerManager::GetLayer(layerMask);

		physx::PxFilterData data{};
		data.word0 = layer.bitValue;

		physx::PxQueryFilterData qFilterData;
		qFilterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;
		qFilterData.data = data;

		std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS> buffer;
		Vector<Entity> tempEnts;
		uint32_t count;

		bool hit = OverlapGeometry(origin, physx::PxBoxGeometry(halfSize.x, halfSize.y, halfSize.z), buffer, count, qFilterData);
		if (!buffer.empty())
		{
			for (auto& overlap : buffer)
			{
				if (overlap.actor != nullptr)
				{
					auto actor = (PhysicsActorBase*)overlap.actor->userData;
					if (actor)
					{
						tempEnts.push_back(actor->GetEntity());
					}
				}
			}
		}

		hitList = tempEnts;

		return hit;
	}

	bool PhysicsScene::OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, Vector<Entity>& hitList, uint32_t layerMask)
	{
		const auto& layer = PhysicsLayerManager::GetLayer(layerMask);

		physx::PxFilterData data{};
		data.word0 = layer.bitValue;

		physx::PxQueryFilterData qFilterData;
		qFilterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;
		qFilterData.data = data;

		std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS> buffer;
		Vector<Entity> tempEnts;
		uint32_t count;

		bool hasHit = OverlapGeometry(origin, physx::PxCapsuleGeometry(radius, halfHeight), buffer, count, qFilterData);
		if (!buffer.empty())
		{
			for (auto& hit : buffer)
			{
				if (hit.actor != nullptr)
				{
					auto actor = (PhysicsActorBase*)hit.actor->userData;
					if (actor)
					{
						tempEnts.push_back(actor->GetEntity());
					}
				}
			}
		}

		hitList = tempEnts;
		return hasHit;
	}

	bool PhysicsScene::OverlapSphere(const glm::vec3& origin, float radius, Vector<Entity>& hitList, uint32_t layerMask)
	{
		physx::PxFilterData data{};
		data.word0 = layerMask;

		physx::PxQueryFilterData qFilterData;
		qFilterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;
		qFilterData.data = data;

		std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS> buffer;
		Vector<Entity> tempEnts;
		uint32_t count;

		bool hasHit = OverlapGeometry(origin, physx::PxSphereGeometry(radius), buffer, count, qFilterData);
		if (!buffer.empty())
		{
			for (auto& hit : buffer)
			{
				if (hit.actor != nullptr)
				{
					auto actor = (PhysicsActorBase*)hit.actor->userData;
					if (actor)
					{
						tempEnts.push_back(actor->GetEntity());
					}
				}
			}
		}

		hitList = tempEnts;
		return hasHit;
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
			m_physXScene->addBroadPhaseRegion(region);
		}
	}

	bool PhysicsScene::Advance(float timeStep)
	{
		VT_PROFILE_FUNCTION();

		SubstepStrategy(timeStep);

		for (uint32_t i = 0; i < m_numSubSteps; i++)
		{
			m_inSimulation = true;
			m_physXScene->simulate(m_subStepSize);
			m_physXScene->fetchResults(true);
		}

		m_inSimulation = false;

		return m_numSubSteps != 0;
	}

	void PhysicsScene::SubstepStrategy(float timeStep)
	{
		if (m_accumulator > m_subStepSize)
		{
			m_accumulator = 0.f;
		}

		m_accumulator += timeStep;
		if (m_accumulator < m_subStepSize)
		{
			m_numSubSteps = 0;
			return;
		}

		m_numSubSteps = glm::min(static_cast<uint32_t>(m_accumulator / m_subStepSize), m_maxSubSteps);
		m_accumulator -= (float)m_numSubSteps * m_subStepSize;
	}

	void PhysicsScene::Destroy()
	{
		VT_ASSERT_MSG(m_physXScene, "Trying to destroy invalid physics scene!");

		m_inSimulation = true;
		for (int32_t i = (int32_t)m_physicsActors.size() - 1; i >= 0; --i)
		{
			RemoveActor(m_physicsActors[i]);
		}
		m_physicsActors.clear();

		for (const auto& f : m_functionQueue)
		{
			f();
		}

		m_functionQueue.clear();

		m_physXScene->release();
		m_physXScene = nullptr;
	}

	bool PhysicsScene::OverlapGeometry(const glm::vec3& origin, const physx::PxGeometry& geometry, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count, const physx::PxQueryFilterData& filterData)
	{
		physx::PxOverlapBuffer buf(buffer.data(), MAX_OVERLAP_COLLIDERS);
		physx::PxTransform pose = PhysXUtilities::ToPhysXTransform(glm::translate(glm::mat4(1.0f), origin));

		bool result = m_physXScene->overlap(geometry, pose, buf, filterData);
		if (result)
		{
			memcpy(buffer.data(), buf.touches, buf.nbTouches * sizeof(physx::PxOverlapHit));
			count = buf.nbTouches;
		}

		return result;
	}
}
