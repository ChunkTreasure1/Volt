#pragma once

#include <EntitySystem/EntityID.h>

#include <CoreUtilities/Containers/Vector.h>

#include <glm/glm.hpp>

namespace Volt
{
	typedef uint32_t WorldCellID;

	static constexpr WorldCellID INVALID_WORLD_CELL_ID = std::numeric_limits<uint32_t>::max();

	struct WorldCell
	{
		WorldCellID cellId = 0;
		Vector<EntityID> cellEntities;
		glm::ivec3 origin;

		bool isLoaded = false;
	};
}
