#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	class Framebuffer;
	class Shader;
	class Camera;

	struct RenderPass
	{
		Ref<Framebuffer> framebuffer;
		Ref<Shader> overrideShader;

		size_t exclusiveShaderHash = 0;
		std::vector<size_t> excludedShaderHashes;
		std::string debugName = "Default";
	};
}