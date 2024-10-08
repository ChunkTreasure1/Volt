#pragma once
#include "Circuit/Config.h"
#include "Circuit/CircuitColor.h"

#include <RHIModule/Descriptors/ResourceHandle.h>

#include <glm/vec2.hpp>

namespace Circuit
{
	enum class CircuitPrimitiveType : uint32_t
	{
		Circle = 0,
		Rect = 1,
		TextCharacter = 2
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
		Volt::ResourceHandle texture;
		glm::vec2 padding;

		glm::vec4 minMaxUV;
		glm::vec4 minMaxPx;
	};
};
