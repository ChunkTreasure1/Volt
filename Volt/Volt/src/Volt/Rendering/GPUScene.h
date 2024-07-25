#pragma once

#include <VoltRHI/Descriptors/ResourceHandle.h>

#include <glm/glm.hpp>
#include <cstdint>

namespace Volt
{
	struct GPUMesh
	{
		inline static constexpr uint32_t MAX_LOD_COUNT = 8;

		ResourceHandle vertexPositionsBuffer;
		ResourceHandle vertexMaterialBuffer;
		ResourceHandle vertexAnimationInfoBuffer;
		ResourceHandle vertexBoneInfluencesBuffer;

		ResourceHandle vertexBoneWeightsBuffer;
		ResourceHandle meshletDataBuffer;
		ResourceHandle meshletsBuffer;
		ResourceHandle sdfBuffer;

		glm::vec3 center;
		float radius;

		uint32_t sdfStartOffset;
		uint32_t vertexStartOffset;
		uint32_t meshletCount;
		uint32_t meshletStartOffset;
		uint32_t meshletIndexStartOffset;
	};

	struct PrimitiveDrawData
	{
		glm::quat rotation;
		glm::vec3 position;
		glm::vec3 scale;

		uint32_t meshId;
		uint32_t materialId;
		uint32_t meshletStartOffset;
		uint32_t entityId;

		uint32_t isAnimated;
		uint32_t boneOffset;
	};

	struct GPUMaterial
	{
		ResourceHandle textures[16];
		ResourceHandle samplers[16];

		uint32_t textureCount = 0;
		uint32_t materialFlags = 0;
		glm::uvec2 padding;
	};

	struct GPUScene
	{
		ResourceHandle meshesBuffer;
		ResourceHandle materialsBuffer;
		ResourceHandle primitiveDrawDataBuffer;
		ResourceHandle meshletsBuffer;
		ResourceHandle bonesBuffer;
	};
}
