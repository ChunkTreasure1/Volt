#pragma once
#include "Circuit/CircuitCoreDefines.h"
#include "Circuit/CircuitColor.h"

#include <glm/vec2.hpp>

#include <cstdint>


namespace Circuit
{
	enum class CircuitPrimitiveType : uint32_t
	{
		Circle,
		Rect
	};
	struct CircuitDrawCommand
	{
		CircuitPrimitiveType type;
		uint32_t primitiveGroup;
		float rotation;
		float scale;
		glm::vec2 radiusHalfSize;
		glm::vec2 pixelPos;
		CircuitColor color;
	};
};
