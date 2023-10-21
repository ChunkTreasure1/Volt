#pragma once

#include <glm/glm.hpp>

namespace Volt
{
	struct SamplersData
	{
		uint32_t linearSampler;
		uint32_t linearPointSampler;

		uint32_t pointSampler;
		uint32_t pointLinearSampler;

		uint32_t linearClampSampler;
		uint32_t linearPointClampSampler;

		uint32_t pointClampSampler;
		uint32_t pointLinearClampSampler;

		uint32_t anisotropicSampler;

		glm::uvec3 padding;
	};
}
