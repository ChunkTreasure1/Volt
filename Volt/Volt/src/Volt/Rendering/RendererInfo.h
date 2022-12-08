#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include <GEM/gem.h>
#include <vector>

namespace Volt
{
	class Mesh;
	class SubMaterial;

	struct RenderCommand
	{
		SubMesh subMesh;
		gem::mat4 transform;

		std::vector<gem::mat4> boneTransforms;

		Ref<Mesh> mesh;
		Ref<SubMaterial> material;

		float timeSinceCreation = 0.f;
		uint32_t id = 0;
		uint32_t objectBufferId = 0;

		bool castShadows = true;
		bool castAO = true;
	};

	enum class DepthState : uint32_t
	{
		ReadWrite = 0,
		Read = 1,
		None = 2
	};

	enum class RasterizerState : uint32_t
	{
		CullBack = 0,
		CullFront,
		CullNone
	};
}