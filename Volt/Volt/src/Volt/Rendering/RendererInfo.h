#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/RendererStructs.h"

#include <GEM/gem.h>
#include <vector>

namespace Volt
{
	class Mesh;
	class SubMaterial;

	struct SubmitCommand
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

	struct InstancedRenderCommand
	{
		SubMesh subMesh;

		std::vector<gem::mat4> boneTransforms;
		std::vector<InstanceData> objectDataIds;

		Ref<Mesh> mesh;
		Ref<SubMaterial> material;

		uint32_t count = 0;
	};
}