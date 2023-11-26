#pragma once

#include "Volt/RenderingNew/Resources/ResourceHandle.h"

#include <glm/glm.hpp>

namespace Volt
{
	struct SamplersData
	{
		ResourceHandle linearSampler;
		ResourceHandle linearPointSampler;

		ResourceHandle pointSampler;
		ResourceHandle pointLinearSampler;

		ResourceHandle linearClampSampler;
		ResourceHandle linearPointClampSampler;

		ResourceHandle pointClampSampler;
		ResourceHandle pointLinearClampSampler;

		ResourceHandle anisotropicSampler;
	};
}
