#pragma once

#include <glm/glm.hpp>

namespace Volt
{
	namespace AI
	{
		enum PolyAreas
		{
			POLYAREA_GROUND,
			POLYAREA_WATER,
			POLYAREA_ROAD,
			POLYAREA_DOOR,
			POLYAREA_GRASS,
			POLYAREA_JUMP
		};

		enum PolyFlags
		{
			POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
			POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
			POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
			POLYFLAGS_JUMP = 0x08,		// Ability to jump.
			POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
			POLYFLAGS_ALL = 0xffff	// All abilities.
		};

		struct NavLinkConnection
		{
			glm::vec3 start = { 0.f };
			glm::vec3 end = { 0.f };
			bool bidirectional = true;
			bool active = true;
		};
	}
}
