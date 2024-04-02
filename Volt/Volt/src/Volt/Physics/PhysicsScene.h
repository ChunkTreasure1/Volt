#pragma once

#include "Volt/Physics/PhysicsSettings.h"
#include "Volt/Physics/PhysicsActor.h"
#include "Volt/Physics/ContactListener.h"

#include "Volt/Physics/PhysXUtilities.h"
#include "Volt/Physics/PhysicsControllerActor.h"

namespace Volt
{
	constexpr uint32_t MAX_OVERLAP_COLLIDERS = 10;

	struct RaycastHit
	{
		EntityID hitEntity;
		glm::vec3 position;
		glm::vec3 normal;
		float distance;
	};

	class PhysicsScene
	{
	public:
		PhysicsScene(const PhysicsSettings& physicsSettings, Scene* entityScene);
		~PhysicsScene();

		void Simulate(float timeStep);

		Ref<PhysicsActor> GetActor(Entity entity);
		const Ref<PhysicsActor> GetActor(Entity entity) const;

		Ref<PhysicsControllerActor> GetControllerActor(Entity entity);
		const Ref<PhysicsControllerActor> GetControllerActor(Entity entity) const;

		Ref<PhysicsActor> CreateActor(Entity entity);
		void RemoveActor(Ref<PhysicsActor> actor);

		Ref<PhysicsControllerActor> CreateControllerActor(Entity entity);
		void RemoveControllerActor(Ref<PhysicsControllerActor> controllerActor);

		inline const glm::vec3 GetGravity() const { return PhysXUtilities::FromPhysXVector(myPhysXScene->getGravity()); }
		inline void SetGravity(const glm::vec3& gravity) { myPhysXScene->setGravity(PhysXUtilities::ToPhysXVector(gravity)); }

		bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit);
		bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit, uint32_t layerMask);

		bool Linecast(const glm::vec3& origin, const glm::vec3& destination, RaycastHit* outHit);
		bool Linecast(const glm::vec3& origin, const glm::vec3& destination, RaycastHit* outHit, uint32_t layerMask);

		bool OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::vector<Entity>& buffer, uint32_t layerMask = 0);
		bool OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::vector<Entity>& buffer, uint32_t layerMask = 0);
		bool OverlapSphere(const glm::vec3& origin, float radius, std::vector<Entity>& buffer, uint32_t layerMask = 0);

		inline const bool IsValid() const { return myPhysXScene != nullptr; }

	private:
		friend class Physics;

		void CreateRegions();

		bool Advance(float timeStep);
		void SubstepStrategy(float timeStep);

		void Destroy();
		bool OverlapGeometry(const glm::vec3& origin, const physx::PxGeometry& geometry, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count, const physx::PxQueryFilterData& filterData);

		physx::PxScene* myPhysXScene = nullptr;
		physx::PxControllerManager* myControllerManager = nullptr;

		std::vector<Ref<PhysicsActor>> myPhysicsActors;
		std::vector<Ref<PhysicsControllerActor>> myControllerActors;

		std::unordered_map<entt::entity, Ref<PhysicsActor>> m_physicsActorFromEntityIDMap;
		std::unordered_map<entt::entity, Ref<PhysicsControllerActor>> m_physicsControllerActorFromEntityIDMap;

		std::vector<std::function<void()>> myFunctionQueue;

		Scene* myEntityScene;

		float mySubStepSize = 0.f;
		float myAccumulator = 0.f;
		uint32_t myNumSubSteps = 0;
		const uint32_t myMaxSubSteps = 8;

		float myUpdateAccumulator = 0.f;
		bool myInSimulation = false;

		inline static ContactListener myContactListener;
	};
}
