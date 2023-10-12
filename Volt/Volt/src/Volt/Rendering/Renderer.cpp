#include "vtpch.h"
#include "Renderer.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Window.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Asset/Rendering/PostProcessingMaterial.h"

#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/VulkanRenderPass.h"
#include "Volt/Rendering/VulkanFramebuffer.h"
#include "Volt/Rendering/Vertex.h"

#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Rendering/Buffer/UniformBufferSet.h"
#include "Volt/Rendering/Buffer/ShaderStorageBuffer.h"
#include "Volt/Rendering/Buffer/ShaderStorageBufferSet.h"

#include "Volt/Rendering/Texture/TextureTable.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Image3D.h"

#include "Volt/Rendering/SamplerStateSet.h"

#include "Volt/Rendering/GlobalDescriptorSetManager.h"

#include "Volt/Utility/FunctionQueue.h"
#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	void Renderer::Initialize()
	{
	}
	void Renderer::LateInitialize()
	{
	}
	void Renderer::Shutdown()
	{
	}
	void Renderer::FlushResourceQueues()
	{
	}
	void Renderer::Flush()
	{
	}
	void Renderer::UpdateDescriptors()
	{
	}
	void Renderer::Begin(Ref<CommandBuffer> commandBuffer)
	{
	}
	void Renderer::End()
	{
	}
	void Renderer::BeginPass(Ref<CommandBuffer> commandBuffer, Ref<VulkanRenderPass> renderPass)
	{
	}
	void Renderer::EndPass(Ref<CommandBuffer> commandBuffer)
	{
	}
	void Renderer::BeginFrameGraphPass(Ref<CommandBuffer> commandBuffer, const FrameGraphRenderPassInfo& renderPassInfo, const FrameGraphRenderingInfo& renderingInfo)
	{
	}
	void Renderer::EndFrameGraphPass(Ref<CommandBuffer> commandBuffer)
	{
	}
	void Renderer::BeginSection(Ref<CommandBuffer> commandBuffer, std::string_view sectionName, const glm::vec4& sectionColor)
	{
	}
	void Renderer::EndSection(Ref<CommandBuffer> commandBuffer)
	{
	}
	void Renderer::SubmitResourceChange(std::function<void()>&& function)
	{
	}
	void Renderer::DrawIndirectBatch(Ref<CommandBuffer> commandBuffer, Ref<ShaderStorageBuffer> indirectCommandBuffer, VkDeviceSize commandOffset, Ref<ShaderStorageBuffer> countBuffer, VkDeviceSize countOffset, const IndirectBatch& batch)
	{
	}
	void Renderer::DrawIndirectBatches(Ref<CommandBuffer> commandBuffer, const IndirectPass& indirectPass, const GlobalDescriptorMap& globalDescriptorSet, const std::vector<IndirectBatch>& batches, const PushConstantDrawData& pushConstantData)
	{
	}
	void Renderer::DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData)
	{
	}
	void Renderer::DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData)
	{
	}
	void Renderer::DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, uint32_t subMeshIndex, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData)
	{
	}
	void Renderer::DrawParticleBatches(Ref<CommandBuffer> commandBuffer, Ref<ShaderStorageBufferSet> storageBufferSet, const GlobalDescriptorMap& globalDescriptorSet, const std::vector<ParticleBatch>& batches)
	{
	}
	void Renderer::DrawFullscreenTriangleWithMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet)
	{
	}
	void Renderer::DrawFullscreenQuadWithMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet)
	{
	}
	void Renderer::DrawVertexBuffer(Ref<CommandBuffer> commandBuffer, uint32_t count, Ref<VertexBuffer> vertexBuffer, Ref<RenderPipeline> pipeline, const GlobalDescriptorMap& globalDescriptorSet)
	{
	}
	void Renderer::DrawIndexedVertexBuffer(Ref<CommandBuffer> commandBuffer, uint32_t indexCount, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<RenderPipeline> pipeline, const void* pushConstantData, uint32_t pushConstantSize)
	{
	}
	void Renderer::DispatchComputePipeline(Ref<CommandBuffer> commandBuffer, Ref<ComputePipeline> pipeline, uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
	}
	void Renderer::ClearImage(Ref<CommandBuffer> commandBuffer, Ref<Image2D> image, const glm::vec4& clearValue)
	{
	}
	SceneEnvironment Renderer::GenerateEnvironmentMap(AssetHandle textureHandle)
	{
		return SceneEnvironment();
	}
	const uint32_t Renderer::AddTexture(Ref<Image2D> image)
	{
		return 0;
	}
	void Renderer::RemoveTexture(Ref<Image2D> image)
	{
	}
	void Renderer::ReloadShader(Ref<Shader> shader)
	{
	}
	void Renderer::AddShaderDependency(Ref<Shader> shader, RenderPipeline* renderPipeline)
	{
	}
	void Renderer::AddShaderDependency(Ref<Shader> shader, ComputePipeline* computePipeline)
	{
	}
	void Renderer::AddShaderDependency(Ref<Shader> shader, SubMaterial* subMaterial)
	{
	}
	void Renderer::AddShaderDependency(Ref<Shader> shader, PostProcessingMaterial* subMaterial)
	{
	}
	void Renderer::RemoveShaderDependency(Ref<Shader> shader, RenderPipeline* renderPipeline)
	{
	}
	void Renderer::RemoveShaderDependency(Ref<Shader> shader, ComputePipeline* computePipeline)
	{
	}
	void Renderer::RemoveShaderDependency(Ref<Shader> shader, SubMaterial* material)
	{
	}
	void Renderer::RemoveShaderDependency(Ref<Shader> shader, PostProcessingMaterial* material)
	{
	}
	const uint32_t Renderer::AddMaterial(SubMaterial* material)
	{
		return 0;
	}
	void Renderer::RemoveMaterial(SubMaterial* material)
	{
	}
	void Renderer::UpdateMaterial(SubMaterial* material)
	{
	}
	Renderer::BindlessData& Renderer::GetBindlessData()
	{
		static Renderer::BindlessData b;
		return b;
		// TODO: insert return statement here
	}
	Renderer::DefaultData& Renderer::GetDefaultData()
	{
		static Renderer::DefaultData d;
		return d;
		// TODO: insert return statement here
	}
	const VulkanFunctions& Renderer::GetVulkanFunctions()
	{
		static VulkanFunctions d;
		return d;
		// TODO: insert return statement here
	}
	const uint32_t Renderer::GetFramesInFlightCount()
	{
		return 0;
	}
	VkDescriptorPool Renderer::GetCurrentDescriptorPool()
	{
		return VkDescriptorPool();
	}
	VkDescriptorPool Renderer::GetDescriptorPool(uint32_t index)
	{
		return VkDescriptorPool();
	}
	VkDescriptorSet Renderer::AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		return VkDescriptorSet();
	}
	VkDescriptorSet Renderer::AllocatePersistentDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		return VkDescriptorSet();
	}
	void Renderer::CreateBindlessData()
	{
	}
	void Renderer::CreateDescriptorPools()
	{
	}
	void Renderer::CreateGlobalDescriptorSets()
	{
	}
	void Renderer::CreateDefaultData()
	{
	}
	void Renderer::GenerateBRDFLut()
	{
	}
	void Renderer::LoadVulkanFunctions()
	{
	}
	void Renderer::LoadRayTracingFunctions()
	{
	}
	void Renderer::DestroyDescriptorPools()
	{
	}
	void Renderer::UpdateMaterialDescriptors()
	{
	}
	void Renderer::UpdateMeshDescriptors()
	{
	}
	void Renderer::UpdateSamplerStateDescriptors()
	{
	}
	void Renderer::SetupPipelineForRendering(Ref<CommandBuffer> commandBuffer, Ref<RenderPipeline> renderPipeline, const GlobalDescriptorMap& globalDescriptorSets, Ref<GlobalDescriptorSet> drawBufferSet)
	{
	}
	void Renderer::SetupPipelineForRendering(Ref<CommandBuffer> commandBuffer, Ref<RenderPipeline> renderPipeline)
	{
	}
}
