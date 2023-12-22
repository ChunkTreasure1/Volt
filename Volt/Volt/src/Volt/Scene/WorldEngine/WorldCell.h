#pragma once

#include "Volt/Scene/EntityID.h"

#include <glm/glm.hpp>

#include <vector>

namespace Volt
{
	typedef uint32_t WorldCellID;

	static constexpr WorldCellID INVALID_WORLD_CELL_ID = std::numeric_limits<uint32_t>::max();

	struct WorldCell
	{
		WorldCellID cellId = 0;
		std::vector<EntityID> cellEntities;
		glm::ivec3 origin;

		bool isLoaded = false;
	};
}
