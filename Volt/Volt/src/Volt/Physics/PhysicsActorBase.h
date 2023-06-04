#pragma once

#include "Volt/Scene/Entity.h"

#include <PhysX/PxPhysics.h>

namespace Volt
{
	class PhysicsActorBase
	{
	public:
		inline PhysicsActorBase(Entity entity)
			: myEntity(entity)
		{
		}

		virtual void SetSimulationData(uint32_t layerId) = 0;

		inline const physx::PxFilterData& GetFilterData() const { return myFilterData; }
		inline const uint32_t GetLayerId() const { return myLayerId; }
		inline Entity GetEntity() const { return myEntity; }

	protected:
		virtual void SynchronizeTransform() = 0;

		physx::PxFilterData myFilterData;
		uint32_t myLayerId = 0;
		Entity myEntity;
	};
}