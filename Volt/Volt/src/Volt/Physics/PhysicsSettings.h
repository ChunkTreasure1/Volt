#pragma once

#include "Volt/Physics/PhysicsEnums.h"

#include <gem/gem.h>

namespace Volt
{
	struct PhysicsSettings
	{
		float fixedTimestep = 1.f / 60.f;
		gem::vec3 gravity = { 0.f, -981.f, 0.f };

		BroadphaseType broadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
		FrictionType frictionModel = FrictionType::Patch;

		gem::vec3 worldBoundsMin = gem::vec3{ -100000.f };
		gem::vec3 worldBoundsMax = gem::vec3{ 100000.f };

		uint32_t worldBoundsSubDivisions = 2;
		uint32_t solverIterations = 8;
		uint32_t solverVelocityIterations = 2;

#ifndef VT_DIST
		bool debugOnPlay = true;
		DebugType debugType = DebugType::LiveDebug;
#endif
	};
}