#pragma once

#include "Volt/RenderingNew/Resources/ResourceHandle.h"

#include <glm/glm.hpp>
#include <cstdint>

namespace Volt
{
	struct GPUMeshLOD
	{
		uint32_t indexCount = 0;
		uint32_t indexOffset = 0;
	};

	struct GPUMesh
	{
		inline static constexpr uint32_t MAX_LOD_COUNT = 8;

		ResourceHandle vertexPositionsBuffer;
		ResourceHandle vertexMaterialBuffer;
		ResourceHandle vertexAnimationBuffer;
		ResourceHandle indexBuffer;

		ResourceHandle meshletIndexBuffer;
		ResourceHandle meshletsBuffer;

		uint32_t vertexStartOffset;
		uint32_t meshletCount;
		uint32_t meshletStartOffset;
		uint32_t meshletIndexStartOffset;

		uint32_t lodCount = 0;
		GPUMeshLOD lods[MAX_LOD_COUNT];
	};

	struct ObjectDrawData
	{
		glm::mat4 transform;
		
		uint32_t meshId;
		uint32_t materialId;
		uint32_t meshletStartOffset;
		uint32_t padding;

		glm::vec3 boundingSphereCenter;
		float boundingSphereRadius;
	};

	struct GPUMaterialNew
	{
		uint32_t textures[16];
		uint32_t samplers[16];

		uint32_t textureCount = 0;
		uint32_t materialFlags = 0;
		glm::uvec2 padding;
	};

	struct GPUScene
	{
		ResourceHandle meshesBuffer;
		ResourceHandle materialsBuffer;
		ResourceHandle objectDrawDataBuffer;
		ResourceHandle meshletsBuffer;
	};
}
