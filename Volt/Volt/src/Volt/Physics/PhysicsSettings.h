#pragma once

#include "Volt/Physics/PhysicsEnums.h"

#include <glm/glm.hpp>

namespace Volt
{
	struct PhysicsSettings
	{
		float fixedTimestep = 1.f / 60.f;
		glm::vec3 gravity = { 0.f, -981.f, 0.f };

		BroadphaseType broadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
		FrictionType frictionModel = FrictionType::Patch;

		glm::vec3 worldBoundsMin = glm::vec3{ -100000.f };
		glm::vec3 worldBoundsMax = glm::vec3{ 100000.f };

		uint32_t worldBoundsSubDivisions = 2;
		uint32_t solverIterations = 8;
		uint32_t solverVelocityIterations = 2;

		bool debugOnPlay = true;
		DebugType debugType = DebugType::LiveDebug;
	};
}
