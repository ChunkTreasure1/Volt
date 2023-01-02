#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	class Framebuffer;
	class Shader;
	class Camera;

	enum class DepthState : uint32_t
	{
		ReadWrite = 0,
		Read = 1,
		None = 2
	};

	enum class CullState : uint32_t
	{
		CullBack = 0,
		CullFront,
		CullNone
	};

	enum class Topology : uint32_t
	{
		None = 0,
		TriangleList = 4,
		PointList = 1,
		LineList = 2
	};

	struct RenderPass
	{
		Ref<Framebuffer> framebuffer;
		Ref<Shader> overrideShader;

		DepthState depthState = DepthState::ReadWrite;
		CullState cullState = CullState::CullBack;

		size_t exclusiveShaderHash = 0;
		std::vector<size_t> excludedShaderHashes;
		std::string debugName = "Default";
	};
}