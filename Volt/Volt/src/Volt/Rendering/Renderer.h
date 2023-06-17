#pragma once

#include "Volt/Core/Buffer.h"

#include "Volt/Rendering/GlobalDescriptorSet.h"
#include "Volt/Rendering/RendererStructs.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"

#include "Volt/Scene/Scene.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class CombinedVertexBuffer;
	class CombinedIndexBuffer;
	class TextureTable;
	class MaterialTable;
	class MeshTable;

	class RenderPipeline;
	class ComputePipeline;
	class Mesh;
	class Material;
	class PostProcessingMaterial;

	class ComputePipeline;
	class CommandBuffer;
	class Shader;
	class Image2D;
	class Image3D;
	class Texture2D;

	class UniformBufferSet;
	class ShaderStorageBuffer;
	class ShaderStorageBufferSet;
	class GlobalDescriptorSet;
	class SamplerStateSet;
	class VertexBuffer;
	class IndexBuffer;

	struct VulkanRenderPass;

	using GlobalDescriptorMap = std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>>;

	struct VulkanFunctions
	{
		PFN_vkCmdBeginDebugUtilsLabelEXT cmdBeginDebugUtilsLabel;
		PFN_vkCmdEndDebugUtilsLabelEXT cmdEndDebugUtilsLabel;

		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
		PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
	};

	class Renderer
	{
	public:
		inline static constexpr uint32_t MAX_MATERIALS = 10000;
		inline static constexpr uint32_t MAX_MESHES = 10000;

		struct BindlessData
		{
			inline static constexpr uint32_t START_TRIANGLE_COUNT = 32768;
			inline static constexpr uint32_t START_VERTEX_COUNT = START_TRIANGLE_COUNT * 3;
			inline static constexpr uint32_t START_INDEX_COUNT = START_TRIANGLE_COUNT * 3;

			Ref<CombinedVertexBuffer> combinedVertexBuffer;
			Ref<CombinedIndexBuffer> combinedIndexBuffer;

			Ref<TextureTable> textureTable;
			Ref<MaterialTable> materialTable;
			Ref<MeshTable> meshTable;
			Ref<SamplerStateSet> samplerStateSet;

			GlobalDescriptorMap globalDescriptorSets;
		};

		struct DefaultData
		{
			Ref<SubMaterial> defaultMaterial;
			Ref<RenderPipeline> defaultRenderPipeline;
			Ref<Shader> defaultShader;
			Ref<Shader> defaultPostProcessingShader;

			Ref<Image2D> brdfLut;

			Ref<Texture2D> whiteTexture;
			Ref<Texture2D> whiteUINTTexture;
			Ref<Texture2D> emptyNormal;
			Ref<Texture2D> emptyMaterial;
		
			Ref<Image3D> black3DImage;
		};

		static void Initialize();
		static void LateInitialize();
		static void Shutdown();
		static void FlushResourceQueues();
		static void Flush();
		static void UpdateDescriptors();

		static void Begin(Ref<CommandBuffer> commandBuffer);
		static void End();

		static void BeginPass(Ref<CommandBuffer> commandBuffer, Ref<VulkanRenderPass> renderPass);
		static void EndPass(Ref<CommandBuffer> commandBuffer);

		static void BeginFrameGraphPass(Ref<CommandBuffer> commandBuffer, const FrameGraphRenderPassInfo& renderPassInfo, const FrameGraphRenderingInfo& renderingInfo);
		static void EndFrameGraphPass(Ref<CommandBuffer> commandBuffer);

		static void BeginSection(Ref<CommandBuffer> commandBuffer, std::string_view sectionName, const glm::vec4& sectionColor);
		static void EndSection(Ref<CommandBuffer> commandBuffer);

		static void SubmitResourceChange(std::function<void()>&& function);

		static void DrawIndirectBatch(Ref<CommandBuffer> commandBuffer, Ref<ShaderStorageBuffer> indirectCommandBuffer, VkDeviceSize commandOffset, Ref<ShaderStorageBuffer> countBuffer, VkDeviceSize countOffset, const IndirectBatch& batch);
		static void DrawIndirectBatches(Ref<CommandBuffer> commandBuffer, const IndirectPass& indirectPass, const GlobalDescriptorMap& globalDescriptorSet, const std::vector<IndirectBatch>& batches, const PushConstantDrawData& pushConstantData = {});

		static void DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData = {});
		static void DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData = {});
		static void DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, uint32_t subMeshIndex, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData = {});

		static void DrawParticleBatches(Ref<CommandBuffer> commandBuffer, Ref<ShaderStorageBufferSet> storageBufferSet, const GlobalDescriptorMap& globalDescriptorSet, const std::vector<ParticleBatch>& batches);

		static void DrawFullscreenTriangleWithMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet);
		static void DrawFullscreenQuadWithMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet);

		static void DrawVertexBuffer(Ref<CommandBuffer> commandBuffer, uint32_t count, Ref<VertexBuffer> vertexBuffer, Ref<RenderPipeline> pipeline, const GlobalDescriptorMap& globalDescriptorSet);
		static void DrawIndexedVertexBuffer(Ref<CommandBuffer> commandBuffer, uint32_t indexCount, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<RenderPipeline> pipeline, const void* pushConstantData = nullptr, uint32_t pushConstantSize = 0);

		static void DispatchComputePipeline(Ref<CommandBuffer> commandBuffer, Ref<ComputePipeline> pipeline, uint32_t groupX, uint32_t groupY, uint32_t groupZ);
		static void ClearImage(Ref<CommandBuffer> commandBuffer, Ref<Image2D> image, const glm::vec4& clearValue);

		static SceneEnvironment GenerateEnvironmentMap(AssetHandle textureHandle);

		static const uint32_t AddTexture(Ref<Image2D> image);
		static void RemoveTexture(Ref<Image2D> image);

		static void ReloadShader(Ref<Shader> shader);

		static void AddShaderDependency(Ref<Shader> shader, RenderPipeline* renderPipeline);
		static void AddShaderDependency(Ref<Shader> shader, ComputePipeline* computePipeline);
		static void AddShaderDependency(Ref<Shader> shader, SubMaterial* subMaterial);
		static void AddShaderDependency(Ref<Shader> shader, PostProcessingMaterial* subMaterial);

		static void RemoveShaderDependency(Ref<Shader> shader, RenderPipeline* renderPipeline);
		static void RemoveShaderDependency(Ref<Shader> shader, ComputePipeline* computePipeline);
		static void RemoveShaderDependency(Ref<Shader> shader, SubMaterial* material);
		static void RemoveShaderDependency(Ref<Shader> shader, PostProcessingMaterial* material);

		static const uint32_t AddMaterial(SubMaterial* material);
		static void RemoveMaterial(SubMaterial* material);
		static void UpdateMaterial(SubMaterial* material);

		static const uint32_t AddMesh(Mesh* mesh, const uint32_t subMeshIndex);
		static void RemoveMesh(Mesh* mesh, const uint32_t subMeshIndex);

		static BindlessData& GetBindlessData();
		static DefaultData& GetDefaultData();
		static const VulkanFunctions& GetVulkanFunctions();

		static const uint32_t GetFramesInFlightCount();

		static VkDescriptorPool GetCurrentDescriptorPool();
		static VkDescriptorPool GetDescriptorPool(uint32_t index);

		static VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo);
		static VkDescriptorSet AllocatePersistentDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo);

	private:
		static void CreateBindlessData();
		static void CreateDescriptorPools();
		static void CreateGlobalDescriptorSets();
		static void CreateDefaultData();

		static void GenerateBRDFLut();

		static void LoadVulkanFunctions();
		static void LoadRayTracingFunctions();

		static void DestroyDescriptorPools();

		static void UpdateMaterialDescriptors();
		static void UpdateMeshDescriptors();
		static void UpdateSamplerStateDescriptors();

		static void SetupPipelineForRendering(Ref<CommandBuffer> commandBuffer, Ref<RenderPipeline> renderPipeline, const GlobalDescriptorMap& globalDescriptorSets, Ref<GlobalDescriptorSet> drawBufferSet = nullptr);
		static void SetupPipelineForRendering(Ref<CommandBuffer> commandBuffer, Ref<RenderPipeline> renderPipeline);

		Renderer() = delete;
	};
}
