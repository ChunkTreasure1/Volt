#pragma once

#include "Volt/Scene/Entity.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Physics/PhysicsActorBase.h"

namespace physx
{
	class PxScene;
	class PxControllerManager;
	class PxController;
	class PxCapsuleController;
}

namespace Volt
{
	class PhysicsControllerActor : public PhysicsActorBase
	{
	public:
		PhysicsControllerActor(Entity entity);
		~PhysicsControllerActor();

		void Create(physx::PxControllerManager* controllerManager);
		void Update(float deltaTime);

		const float GetHeight() const;
		const float GetRadius() const;

		void SetHeight(float height);
		void SetRadius(float radius);

		void Move(const gem::vec3& velocity);
		void Jump(float jumpPower);

		void SetPosition(const gem::vec3& position);
		void SetFootPosition(const gem::vec3& position);

		void SetAngularVelocity(const gem::vec3& velocity);
		const gem::vec3 GetAngularVelocity() const;

		void SetLinearVelocity(const gem::vec3& velocity);
		const gem::vec3 GetLinearVelocity() const;

		void SetGravity(const float& gravity) { myGravity = gravity; };
		float GetGravity() { return myGravity; };

		const bool IsGrounded() const;
		const bool IsValid() const;

		const gem::vec3 GetPosition() const;
		const gem::vec3 GetFootPosition() const;

		void SetSimulationData(uint32_t layerId) override;
		void SetOffset(const gem::vec3& offset);

	protected:
		friend class PhysicsScene;

		void SynchronizeTransform() override;

	private:
		void CreateController(physx::PxControllerManager* controllerManager);
		physx::PxCapsuleController* GetCapsuleController();

		uint32_t myCurrentCollisionFlags = 0;
		CharacterControllerComponent myControllerData;

		physx::PxController* myController = nullptr;
		physx::PxMaterial* myMaterial = nullptr;

		bool myHasGravity = true;
		float myDownSpeed = 0.f;
		float myGravity = 0.f;

		float myDeltaTime = 0.f;

		gem::vec3 myMovement = 0.f;
		gem::vec3 myOffset = 0.f;
	};
}