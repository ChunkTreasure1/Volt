#pragma once

#include <glm/glm.hpp>

#include <RHIModule/Images/Image2D.h>

namespace Volt
{
	struct SceneEnvironment
	{
		RefPtr<RHI::Image2D> irradianceMap;
		RefPtr<RHI::Image2D> radianceMap;

		float lod = 0.f;
		float intensity = 1.f;
	};
}
