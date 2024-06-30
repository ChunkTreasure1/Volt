#pragma once
#include "Circuit/CircuitCoreDefines.h"

#include <glm/vec2.hpp>

#include <cstdint>

namespace Circuit
{
	struct CircuitDrawCommand
	{
		uint32_t type;
		uint32_t primitiveGroup;
		float rotation;
		float scale;
		glm::vec2 radiusHalfSize;
		glm::vec2 pixelPos;
	};
};
