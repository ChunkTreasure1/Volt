#pragma once

#include "Volt/Physics/PhysicsSettings.h"
#include "Volt/Physics/PhysicsActor.h"
#include "Volt/Physics/ContactListener.h"

#include "Volt/Physics/PhysXUtilities.h"

namespace Volt
{
	constexpr uint32_t MAX_OVERLAP_COLLIDERS = 10;

	struct RaycastHit
	{
		Wire::EntityId hitEntity;
		gem::vec3 position;
		gem::vec3 normal;
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

		Ref<PhysicsActor> CreateActor(Entity entity);
		void RemoveActor(Ref<PhysicsActor> actor);

		inline const gem::vec3& GetGravity() const { return PhysXUtilities::FromPhysXVector(myPhysXScene->getGravity()); }
		inline void SetGravity(const gem::vec3& gravity) { myPhysXScene->setGravity(PhysXUtilities::ToPhysXVector(gravity)); }

		bool Raycast(const gem::vec3& origin, const gem::vec3& direction, float maxDistance, RaycastHit* outHit);
		bool Raycast(const gem::vec3& origin, const gem::vec3& direction, float maxDistance, RaycastHit* outHit, uint32_t layerMask);

		bool OverlapBox(const gem::vec3& origin, const gem::vec3& halfSize, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count);
		bool OverlapCapsule(const gem::vec3& origin, float radius, float halfHeight, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count);
		bool OverlapSphere(const gem::vec3& origin, float radius, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count);

		inline const bool IsValid() const { return myPhysXScene != nullptr; }

	private:
		friend class Physics;

		void CreateRegions();

		bool Advance(float timeStep);
		void SubstepStrategy(float timeStep);

		void Destroy();
		bool OverlapGeometry(const gem::vec3& origin, const physx::PxGeometry& geometry, std::array<physx::PxOverlapHit, MAX_OVERLAP_COLLIDERS>& buffer, uint32_t& count);

		physx::PxScene* myPhysXScene = nullptr;
		std::vector<Ref<PhysicsActor>> myPhysicsActors;
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