#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/RendererStructs.h"

#include <glm/glm.hpp>
#include <vector>

namespace Volt
{
	class Mesh;
	class SubMaterial;
	class Texture2D;
	class Font;

	class ShaderStorageBufferSet;
	class GlobalDescriptorSet;
	class ComputePipeline;

	struct SubmitCommand
	{
		SubMesh subMesh;
		glm::mat4 transform;

		std::vector<glm::mat4> boneTransforms;
		std::vector<glm::vec4> vertexColors;

		Ref<Mesh> mesh;
		Ref<SubMaterial> material;

		float timeSinceCreation = 0.f;
		float randomValue = 0.f;
		uint32_t id = 0;
		uint32_t objectBufferId = 0;
		uint32_t batchId = 0;
		uint32_t subMeshIndex = 0;
	};

	struct LineCommand
	{
		glm::vec3 startPosition;
		glm::vec3 endPosition;
		glm::vec4 color;
	};

	struct BillboardCommand
	{
		glm::vec4 color;
		glm::vec3 position;
		glm::vec3 scale;

		Ref<Texture2D> texture;
		uint32_t id;
	};

	struct TextCommand
	{
		glm::mat4 transform;
		std::string text;
		glm::vec4 color;
		Ref<Font> font;
		float maxWidth;
	};

	struct DecalCommand
	{
		glm::mat4 transform;
		glm::vec4 color;
		float randomValue;
		float timeSinceCreation;
		
		Ref<Material> material;
	};

	enum class MemoryUsage : uint32_t
	{
		None = 0,
		Indirect,
		CPUToGPU,
		GPUOnly
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(MemoryUsage);

	struct PushConstantDrawData
	{
		const void* data = nullptr;
		const uint32_t size = 0;

		inline const bool IsValid() const
		{
			return data != nullptr;
		}
	};

	struct IndirectPass
	{
		Ref<ShaderStorageBufferSet> drawCountIDStorageBuffer;
		Ref<ShaderStorageBufferSet> drawArgsStorageBuffer;
		Ref<GlobalDescriptorSet> drawBuffersSet;

		Ref<ComputePipeline> indirectCullPipeline;
		Ref<ComputePipeline> clearCountBufferPipeline;
	};
}
