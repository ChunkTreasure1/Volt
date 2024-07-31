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

		glm::vec3 center;
		float radius;

		uint32_t vertexStartOffset;
		uint32_t meshletCount;
		uint32_t meshletStartOffset;
		uint32_t meshletIndexStartOffset;
	};

	struct GPUMeshSDF
	{
		ResourceHandle sdfTexture;
		glm::vec3 size;

		glm::vec3 min;
		glm::vec3 max;
		ResourceHandle bricksBuffer;
		uint32_t brickCount;
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

	struct SDFPrimitiveDrawData
	{
		glm::quat rotation;
		glm::vec3 position;
		glm::vec3 scale;

		uint32_t meshSDFId;
		uint32_t primtiveId;
	};

	struct GPUSDFBrick
	{
		glm::vec3 min;
		glm::vec3 max;

		glm::vec3 localCoord;
	};

	struct GPUMaterial
	{
		ResourceHandle textures[16];
		ResourceHandle samplers[16];

		uint32_t textureCount = 0;
		uint32_t materialFlags = 0;
		glm::uvec2 padding;
	};
}
