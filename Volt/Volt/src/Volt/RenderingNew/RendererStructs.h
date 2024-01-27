#pragma once

#include <glm/glm.hpp>

namespace Volt
{
	namespace RHI
	{
		class Image2D;
	}

	struct SceneEnvironment
	{
		Ref<RHI::Image2D> irradianceMap;
		Ref<RHI::Image2D> radianceMap;

		float lod = 0.f;
		float intensity = 1.f;
	};
}
