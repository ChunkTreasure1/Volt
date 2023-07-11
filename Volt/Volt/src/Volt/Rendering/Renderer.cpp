#include "vtpch.h"
#include "Renderer.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Window.h"
#include "Volt/Core/Graphics/Swapchain.h"
#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Core/Graphics/GraphicsDevice.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Asset/Rendering/PostProcessingMaterial.h"

#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/VulkanRenderPass.h"
#include "Volt/Rendering/VulkanFramebuffer.h"
#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"
#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"
#include "Volt/Rendering/Vertex.h"

#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Rendering/Buffer/UniformBufferSet.h"
#include "Volt/Rendering/Buffer/ShaderStorageBuffer.h"
#include "Volt/Rendering/Buffer/ShaderStorageBufferSet.h"

#include "Volt/Rendering/Buffer/IndexBuffer.h"
#include "Volt/Rendering/Buffer/VertexBuffer.h"

#include "Volt/Rendering/Texture/TextureTable.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Image3D.h"

#include "Volt/Rendering/MaterialTable.h"
#include "Volt/Rendering/MeshTable.h"
#include "Volt/Rendering/SamplerStateSet.h"

#include "Volt/Rendering/GlobalDescriptorSetManager.h"

#include "Volt/Utility/FunctionQueue.h"
#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	struct ShaderDependencies
	{
		std::vector<RenderPipeline*> renderPipelines;
		std::vector<ComputePipeline*> computePipelines;
		std::vector<SubMaterial*> materials;
		std::vector<PostProcessingMaterial*> postProcessingMaterials;
	};

	struct RendererData
	{
		Renderer::BindlessData bindlessData{};
		VulkanFunctions vulkanFunctions;

		std::vector<VkDescriptorPool> perFrameDescriptorPools;
		std::vector<VkDescriptorPool> perFramePersistentDescriptorPools;

		uint32_t framesInFlightCount = 0;
	};

	struct ThreadRenderData
	{
		Ref<RenderPipeline> currentOverridePipeline;
		Ref<VulkanRenderPass> currentRenderPass;

		MaterialFlag currentMaterialFlags = MaterialFlag::All;
	};

	static std::vector<FunctionQueue> myResourceChangeQueues;
	inline static Scope<RendererData> s_rendererData;
	inline static Scope<Renderer::DefaultData> s_defaultData;

	inline static std::mutex s_shaderDependenciesMutex;
	inline static std::mutex s_threadDataMutex;
	inline static std::mutex s_resourceQueueMutex;
	inline static std::mutex s_materialMutex;
	inline static std::mutex s_textureMutex;
	inline static std::mutex s_meshMutex;

	inline static std::unordered_map<size_t, ShaderDependencies> s_shaderDependencies;
	inline static std::unordered_map<std::thread::id, ThreadRenderData> s_renderData;

	inline static ThreadRenderData& GetThreadData()
	{
		std::scoped_lock lock{ s_threadDataMutex };

		const auto threadId = std::this_thread::get_id();
		return s_renderData[threadId];
	}

	void Renderer::Initialize()
	{
		const uint32_t framesInFlight = Application::Get().GetWindow().GetSwapchain().GetMaxFramesInFlight();
		s_rendererData = CreateScope<RendererData>();
		s_rendererData->framesInFlightCount = framesInFlight;

		myResourceChangeQueues.resize(framesInFlight);

		GlobalDescriptorSetManager::Initialize();

#ifdef VT_ENABLE_GPU_MARKERS
		LoadVulkanFunctions();
#endif

		if (GraphicsContext::GetPhysicalDevice()->GetCapabilities().supportsRayTracing)
		{
			LoadRayTracingFunctions();
		}

		CreateBindlessData();
		CreateDescriptorPools();
		CreateGlobalDescriptorSets();

		UpdateSamplerStateDescriptors();
		UpdateMaterialDescriptors();
		UpdateMeshDescriptors();

		CreateDefaultData();
	}

	void Renderer::LateInitialize()
	{
		GenerateBRDFLut();
	}

	void Renderer::Shutdown()
	{
		auto device = GraphicsContext::GetDevice();
		device->WaitForIdle();

		DestroyDescriptorPools();

		GlobalDescriptorSetManager::Shutdown();

		s_defaultData = nullptr;
		s_rendererData = nullptr;
		s_shaderDependencies.clear();
	}

	void Renderer::FlushResourceQueues()
	{
		for (auto& resourceQueue : myResourceChangeQueues)
		{
			resourceQueue.Flush();
		}
	}

	void Renderer::Flush()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

		{
			std::scoped_lock lock{ s_resourceQueueMutex };
			myResourceChangeQueues.at(currentFrame).Flush();
		}

		// Clear descriptor pool
		{
			vkResetDescriptorPool(device->GetHandle(), s_rendererData->perFrameDescriptorPools.at(currentFrame), 0);
		}
	}

	void Renderer::UpdateDescriptors()
	{
		VT_PROFILE_FUNCTION();

		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

		auto& bindlessData = GetBindlessData();
		
		{
			std::scoped_lock lock{ s_materialMutex };
			bindlessData.materialTable->UpdateMaterialData(currentFrame);
		}

		{
			std::scoped_lock lock{ s_meshMutex };
			bindlessData.meshTable->UpdateMeshData(currentFrame);
		}

		{
			std::scoped_lock lock{ s_textureMutex };
		bindlessData.textureTable->UpdateDescriptors(bindlessData.globalDescriptorSets.at(Sets::TEXTURES), currentFrame);
		}
	}

	void Renderer::Begin(Ref<CommandBuffer> commandBuffer)
	{
	}

	void Renderer::End()
	{
	}

	void Renderer::BeginPass(Ref<CommandBuffer> commandBuffer, Ref<VulkanRenderPass> renderPass)
	{
		VT_PROFILE_FUNCTION();

		auto& threadData = GetThreadData();

		threadData.currentRenderPass = renderPass;
		threadData.currentOverridePipeline = renderPass->overridePipeline;
		renderPass->framebuffer->Bind(commandBuffer->GetCurrentCommandBuffer());

		// Begin rendering
		const auto& attachmentInfos = renderPass->framebuffer->GetColorAttachmentInfos();

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = { 0, 0, renderPass->framebuffer->GetWidth(), renderPass->framebuffer->GetHeight() };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = (uint32_t)attachmentInfos.size();
		renderingInfo.pColorAttachments = attachmentInfos.data();
		renderingInfo.pStencilAttachment = nullptr;

		if (renderPass->framebuffer->GetDepthAttachment())
		{
			renderingInfo.pDepthAttachment = &renderPass->framebuffer->GetDepthAttachmentInfo();
		}
		else
		{
			renderingInfo.pDepthAttachment = nullptr;
		}

		vkCmdBeginRendering(commandBuffer->GetCurrentCommandBuffer(), &renderingInfo);
	}

	void Renderer::EndPass(Ref<CommandBuffer> commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		vkCmdEndRendering(commandBuffer->GetCurrentCommandBuffer());

		auto& threadData = GetThreadData();

		threadData.currentRenderPass->framebuffer->Unbind(commandBuffer->GetCurrentCommandBuffer());
		threadData.currentRenderPass = nullptr;
		threadData.currentOverridePipeline = nullptr;
	}

	void Renderer::BeginFrameGraphPass(Ref<CommandBuffer> commandBuffer, const FrameGraphRenderPassInfo& renderPassInfo, const FrameGraphRenderingInfo& renderingInfo)
	{
		VT_PROFILE_FUNCTION();

		auto& threadData = GetThreadData();
		threadData.currentOverridePipeline = renderPassInfo.overridePipeline;
		threadData.currentMaterialFlags = renderPassInfo.materialFlags;

		VkExtent2D extent{};
		extent.width = renderingInfo.width;
		extent.height = renderingInfo.height;

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;

		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor = { { 0, 0 }, extent };
		vkCmdSetViewport(commandBuffer->GetCurrentCommandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer->GetCurrentCommandBuffer(), 0, 1, &scissor);

		VkRenderingInfo vkRenderingInfo{};
		vkRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		vkRenderingInfo.renderArea = { 0, 0, renderingInfo.width, renderingInfo.height };
		vkRenderingInfo.layerCount = renderingInfo.layers;
		vkRenderingInfo.colorAttachmentCount = (uint32_t)renderingInfo.colorAttachmentInfo.size();
		vkRenderingInfo.pColorAttachments = renderingInfo.colorAttachmentInfo.data();
		vkRenderingInfo.pStencilAttachment = nullptr;

		if (renderingInfo.hasDepth)
		{
			vkRenderingInfo.pDepthAttachment = &renderingInfo.depthAttachmentInfo;
		}
		else
		{
			vkRenderingInfo.pDepthAttachment = nullptr;
		}

		vkCmdBeginRendering(commandBuffer->GetCurrentCommandBuffer(), &vkRenderingInfo);
	}

	void Renderer::EndFrameGraphPass(Ref<CommandBuffer> commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		vkCmdEndRendering(commandBuffer->GetCurrentCommandBuffer());

		auto& threadData = GetThreadData();
		threadData.currentOverridePipeline = nullptr;
	}

	void Renderer::BeginSection(Ref<CommandBuffer> commandBuffer, std::string_view sectionName, const glm::vec4& sectionColor)
	{
#ifdef VT_ENABLE_GPU_MARKERS
		VkDebugUtilsLabelEXT markerInfo{};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		markerInfo.pLabelName = sectionName.data();
		markerInfo.color[0] = sectionColor.x;
		markerInfo.color[1] = sectionColor.y;
		markerInfo.color[2] = sectionColor.z;
		markerInfo.color[3] = sectionColor.w;

		s_rendererData->vulkanFunctions.cmdBeginDebugUtilsLabel(commandBuffer->GetCurrentCommandBuffer(), &markerInfo);
#endif
	}

	void Renderer::EndSection(Ref<CommandBuffer> commandBuffer)
	{
#ifdef VT_ENABLE_GPU_MARKERS
		s_rendererData->vulkanFunctions.cmdEndDebugUtilsLabel(commandBuffer->GetCurrentCommandBuffer());
#endif
	}

	void Renderer::SubmitResourceChange(std::function<void()>&& function)
	{
		std::scoped_lock lock{ s_resourceQueueMutex };

		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();
		myResourceChangeQueues.at(currentFrame).Push(std::move(function));
	}

	void Renderer::DrawIndirectBatch(Ref<CommandBuffer> commandBuffer, Ref<ShaderStorageBuffer> indirectCommandBuffer, VkDeviceSize commandOffset, Ref<ShaderStorageBuffer> countBuffer, VkDeviceSize countOffset, const IndirectBatch& batch)
	{
		vkCmdDrawIndexedIndirectCount(commandBuffer->GetCurrentCommandBuffer(), indirectCommandBuffer->GetHandle(), commandOffset, countBuffer->GetHandle(), countOffset, batch.count, sizeof(IndirectGPUCommand));
	}

	void Renderer::DrawIndirectBatches(Ref<CommandBuffer> commandBuffer, const IndirectPass& indirectPass, const GlobalDescriptorMap& globalDescriptorSet, const std::vector<IndirectBatch>& batches, const PushConstantDrawData& pushConstantData)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		auto& threadData = GetThreadData();

		const bool pipelineOverridden = threadData.currentOverridePipeline != nullptr;
		const auto overridePipeline = threadData.currentOverridePipeline;

		if (pipelineOverridden)
		{
			overridePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			if (pushConstantData.IsValid())
			{
				overridePipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), pushConstantData.data, pushConstantData.size);
			}

			SetupPipelineForRendering(commandBuffer, overridePipeline, globalDescriptorSet, indirectPass.drawBuffersSet);
		}

		Ref<SubMaterial> lastMaterial = nullptr;
		Ref<Mesh> lastMesh = nullptr;

		for (size_t i = 0; i < batches.size(); i++)
		{
			const bool hasMaterialFlag = ((batches[i].material->GetFlags() & threadData.currentMaterialFlags) != MaterialFlag::All) || threadData.currentMaterialFlags == MaterialFlag::All;
			if (!hasMaterialFlag)
			{
				continue;
			}

			if (pipelineOverridden)
			{
				const bool isPermutationOfOverridePipeline = batches[i].material->GetPipeline()->IsPermutationOf(overridePipeline);

				if (batches[i].material->GetPipeline()->GetHash() == overridePipeline->GetHash())
				{
					if (i != 0 && lastMaterial->GetPipeline() != batches[i].material->GetPipeline())
					{
						overridePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
						lastMaterial = batches[i].material;
					}

					batches[i].material->PushMaterialData(commandBuffer);
				}
				else if (isPermutationOfOverridePipeline)
				{
					batches[i].material->Bind(commandBuffer);
					batches[i].material->PushMaterialData(commandBuffer);
					SetupPipelineForRendering(commandBuffer, batches[i].material->GetPipeline(), globalDescriptorSet, indirectPass.drawBuffersSet);
				}
			}
			else
			{
				if (!lastMaterial || batches[i].material != lastMaterial)
				{
					batches[i].material->Bind(commandBuffer);
					batches[i].material->PushMaterialData(commandBuffer);
					SetupPipelineForRendering(commandBuffer, batches[i].material->GetPipeline(), globalDescriptorSet, indirectPass.drawBuffersSet);

					lastMaterial = batches[i].material;
				}
			}

			if (!lastMesh || batches[i].mesh != lastMesh)
			{
				batches[i].mesh->GetVertexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());
				batches[i].mesh->GetIndexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());

				lastMesh = batches[i].mesh;
			}

			const VkDeviceSize drawOffset = batches.at(i).first * sizeof(IndirectGPUCommand);
			const VkDeviceSize countOffset = i * sizeof(uint32_t);

			const Ref<ShaderStorageBuffer> currentIndirectBuffer = indirectPass.drawArgsStorageBuffer->Get(Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, currentIndex);
			const Ref<ShaderStorageBuffer> currentCountBuffer = indirectPass.drawCountIDStorageBuffer->Get(Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, currentIndex);

			DrawIndirectBatch(commandBuffer, currentIndirectBuffer, drawOffset, currentCountBuffer, countOffset, batches.at(i));
		}
	}

	void Renderer::DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData)
	{
		VT_PROFILE_FUNCTION();

		auto& threadData = GetThreadData();

		const bool pipelineOverridden = threadData.currentOverridePipeline != nullptr;
		const auto& overridePipeline = threadData.currentOverridePipeline;

		if (pipelineOverridden)
		{
			overridePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			if (pushConstantData.IsValid())
			{
				overridePipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), pushConstantData.data, pushConstantData.size);
			}

			SetupPipelineForRendering(commandBuffer, overridePipeline, globalDescriptorSet);
		}

		mesh->GetVertexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());
		mesh->GetIndexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());

		for (const auto& subMesh : mesh->GetSubMeshes())
		{
			if (!pipelineOverridden)
			{
				auto material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
				material->Bind(commandBuffer);
				material->PushMaterialData(commandBuffer);
				SetupPipelineForRendering(commandBuffer, material->GetPipeline(), globalDescriptorSet);
			}

			vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), subMesh.indexCount, 1, subMesh.indexStartOffset + mesh->GetIndexStartOffset(), subMesh.vertexStartOffset + mesh->GetVertexStartOffset(), 0);
		}
	}

	void Renderer::DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, Ref<Material> overrideMaterial, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData)
	{
		VT_PROFILE_FUNCTION();

		auto& threadData = GetThreadData();

		const bool pipelineOverridden = threadData.currentOverridePipeline != nullptr;
		const auto& overridePipeline = threadData.currentOverridePipeline;

		if (pipelineOverridden)
		{
			overridePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			if (pushConstantData.IsValid())
			{
				overridePipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), pushConstantData.data, pushConstantData.size);
			}

			SetupPipelineForRendering(commandBuffer, overridePipeline, globalDescriptorSet);
		}

		mesh->GetVertexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());
		mesh->GetIndexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());

		for (const auto& subMesh : mesh->GetSubMeshes())
		{
			if (!pipelineOverridden)
			{
				auto material = overrideMaterial->TryGetSubMaterialAt(subMesh.materialIndex);
				material->Bind(commandBuffer);
				material->PushMaterialData(commandBuffer);
				SetupPipelineForRendering(commandBuffer, material->GetPipeline(), globalDescriptorSet);
			}

			vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), subMesh.indexCount, 1, subMesh.indexStartOffset + mesh->GetIndexStartOffset(), subMesh.vertexStartOffset + mesh->GetVertexStartOffset(), 0);
		}
	}

	void Renderer::DrawMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, uint32_t subMeshIndex, const GlobalDescriptorMap& globalDescriptorSet, const PushConstantDrawData& pushConstantData)
	{
		VT_PROFILE_FUNCTION();

		auto& threadData = GetThreadData();

		const bool pipelineOverridden = threadData.currentOverridePipeline != nullptr;
		const auto& overridePipeline = threadData.currentOverridePipeline;

		if (pipelineOverridden)
		{
			overridePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			if (pushConstantData.IsValid())
			{
				overridePipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), pushConstantData.data, pushConstantData.size);
			}

			SetupPipelineForRendering(commandBuffer, overridePipeline, globalDescriptorSet);
		}

		const auto& subMesh = mesh->GetSubMeshes().at(subMeshIndex);

		if (!pipelineOverridden)
		{
			auto material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
			material->Bind(commandBuffer);
			material->PushMaterialData(commandBuffer);
			SetupPipelineForRendering(commandBuffer, material->GetPipeline(), globalDescriptorSet);
		}

		mesh->GetVertexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());
		mesh->GetIndexBuffer()->Bind(commandBuffer->GetCurrentCommandBuffer());

		vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), subMesh.indexCount, 1, subMesh.indexStartOffset + mesh->GetIndexStartOffset(), subMesh.vertexStartOffset + mesh->GetVertexStartOffset(), 0);
	}

	void Renderer::DrawParticleBatches(Ref<CommandBuffer> commandBuffer, Ref<ShaderStorageBufferSet> storageBufferSet, const GlobalDescriptorMap& globalDescriptorSet, const std::vector<ParticleBatch>& batches)
	{
		VT_PROFILE_FUNCTION();

		struct PushConstant
		{
			uint32_t particleOffset = 0;
		} constants;

		for (const auto& batch : batches)
		{
			const auto& material = batch.material->GetSubMaterialAt(0);
			constants.particleOffset = batch.startOffset;

			material->Bind(commandBuffer);
			SetupPipelineForRendering(commandBuffer, material->GetPipeline(), globalDescriptorSet);
			material->GetPipeline()->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &constants, sizeof(PushConstant));

			vkCmdDraw(commandBuffer->GetCurrentCommandBuffer(), 1, (uint32_t)batch.particles.size(), 0, 0);
		}
	}

	void Renderer::DrawFullscreenTriangleWithMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet)
	{
		VT_PROFILE_FUNCTION();

		material->GetSubMaterialAt(0)->Bind(commandBuffer);
		material->GetSubMaterialAt(0)->PushMaterialData(commandBuffer);
		SetupPipelineForRendering(commandBuffer, material->GetSubMaterialAt(0)->GetPipeline(), globalDescriptorSet);

		vkCmdDraw(commandBuffer->GetCurrentCommandBuffer(), 3, 1, 0, 0);
	}

	void Renderer::DrawFullscreenQuadWithMaterial(Ref<CommandBuffer> commandBuffer, Ref<Material> material, const GlobalDescriptorMap& globalDescriptorSet)
	{
		VT_PROFILE_FUNCTION();

		material->GetSubMaterialAt(0)->Bind(commandBuffer);
		material->GetSubMaterialAt(0)->PushMaterialData(commandBuffer);
		SetupPipelineForRendering(commandBuffer, material->GetSubMaterialAt(0)->GetPipeline(), globalDescriptorSet);

		vkCmdDraw(commandBuffer->GetCurrentCommandBuffer(), 6, 1, 0, 0);
	}

	void Renderer::DrawVertexBuffer(Ref<CommandBuffer> commandBuffer, uint32_t count, Ref<VertexBuffer> vertexBuffer, Ref<RenderPipeline> pipeline, const GlobalDescriptorMap& globalDescriptorSet)
	{
		vkCmdSetLineWidth(commandBuffer->GetCurrentCommandBuffer(), 2.f);

		pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
		SetupPipelineForRendering(commandBuffer, pipeline, globalDescriptorSet);

		vertexBuffer->Bind(commandBuffer->GetCurrentCommandBuffer());

		vkCmdDraw(commandBuffer->GetCurrentCommandBuffer(), count, 1, 0, 0);
	}

	void Renderer::DrawIndexedVertexBuffer(Ref<CommandBuffer> commandBuffer, uint32_t indexCount, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<RenderPipeline> pipeline, const void* pushConstantData, uint32_t pushConstantSize)
	{
		pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

		if (pushConstantData && pushConstantSize > 0)
		{
			pipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), pushConstantData, pushConstantSize);
		}

		SetupPipelineForRendering(commandBuffer, pipeline);

		vertexBuffer->Bind(commandBuffer->GetCurrentCommandBuffer());
		indexBuffer->Bind(commandBuffer->GetCurrentCommandBuffer());

		vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), indexCount, 1, 0, 0, 0);
	}

	void Renderer::DispatchComputePipeline(Ref<CommandBuffer> commandBuffer, Ref<ComputePipeline> pipeline, uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		if (pipeline->HasDescriptorSet(Sets::SAMPLERS))
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(currentIndex);
			pipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::SAMPLERS);
		}

		if (pipeline->HasDescriptorSet(Sets::TEXTURES))
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::TEXTURES]->GetOrAllocateDescriptorSet(currentIndex);
			pipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::TEXTURES);
		}

		if (pipeline->HasDescriptorSet(Sets::MAINBUFFERS))
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::MAINBUFFERS]->GetOrAllocateDescriptorSet(currentIndex);
			pipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::MAINBUFFERS);
		}

		pipeline->Clear(currentIndex);
		pipeline->Dispatch(commandBuffer->GetCurrentCommandBuffer(), groupX, groupY, groupZ, currentIndex);
		pipeline->InsertBarriers(commandBuffer->GetCurrentCommandBuffer(), currentIndex);
	}

	void Renderer::ClearImage(Ref<CommandBuffer> commandBuffer, Ref<Image2D> image, const glm::vec4& clearValue)
	{
		VT_PROFILE_FUNCTION();

		if (!Utility::IsDepthFormat(image->GetFormat()))
		{
			VkClearColorValue clearColor{};
			clearColor.float32[0] = clearValue.x;
			clearColor.float32[1] = clearValue.y;
			clearColor.float32[2] = clearValue.z;
			clearColor.float32[3] = clearValue.w;

			VkImageSubresourceRange range{ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
			vkCmdClearColorImage(commandBuffer->GetCurrentCommandBuffer(), image->GetHandle(), image->GetLayout(), &clearColor, 1, &range);
		}
		else
		{
			VkClearDepthStencilValue clearColor{};
			clearColor.depth = 1.f;
			clearColor.stencil = 0;

			VkImageSubresourceRange range{ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
			vkCmdClearDepthStencilImage(commandBuffer->GetCurrentCommandBuffer(), image->GetHandle(), image->GetLayout(), &clearColor, 1, &range);
		}
	}

	SceneEnvironment Renderer::GenerateEnvironmentMap(AssetHandle textureHandle)
	{
		Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		if (!texture || !texture->IsValid())
		{
			return {};
		}

		constexpr uint32_t cubeMapSize = 1024;
		constexpr uint32_t irradianceMapSize = 32;
		constexpr uint32_t conversionThreadCount = 32;

		auto device = GraphicsContext::GetDevice();

		Ref<Image2D> environmentUnfiltered;
		Ref<Image2D> environmentFiltered;
		Ref<Image2D> irradianceMap;

		// Unfiltered - Conversion
		{
			ImageSpecification imageSpec{};
			imageSpec.format = ImageFormat::RGBA32F;
			imageSpec.width = cubeMapSize;
			imageSpec.height = cubeMapSize;
			imageSpec.usage = ImageUsage::Storage;
			imageSpec.layers = 6;
			imageSpec.isCubeMap = true;

			environmentUnfiltered = Image2D::Create(imageSpec);

			Ref<ComputePipeline> conversionPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("EquirectangularToCubemap"), 1, true);

			VkCommandBuffer cmdBuffer = device->GetCommandBuffer(true);
			environmentUnfiltered->TransitionToLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);

			conversionPipeline->Bind(cmdBuffer);
			conversionPipeline->SetImage(environmentUnfiltered, Sets::OTHER, 0, ImageAccess::Write);
			conversionPipeline->SetImage(texture->GetImage(), Sets::OTHER, 1, ImageAccess::Read);
			conversionPipeline->BindDescriptorSet(cmdBuffer, GetBindlessData().globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(0), Sets::SAMPLERS);

			ExecutionBarrierInfo barrierInfo{};
			barrierInfo.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
			barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
			barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

			conversionPipeline->InsertExecutionBarrier(barrierInfo);

			conversionPipeline->Dispatch(cmdBuffer, cubeMapSize / conversionThreadCount, cubeMapSize / conversionThreadCount, 6);
			conversionPipeline->InsertBarriers(cmdBuffer);

			device->FlushCommandBuffer(cmdBuffer);
		}

		// Filtered
		{
			ImageSpecification imageSpec{};
			imageSpec.format = ImageFormat::RGBA32F;
			imageSpec.width = cubeMapSize;
			imageSpec.height = cubeMapSize;
			imageSpec.usage = ImageUsage::Storage;
			imageSpec.layers = 6;
			imageSpec.isCubeMap = true;
			imageSpec.mips = Utility::CalculateMipCount(cubeMapSize, cubeMapSize);

			environmentFiltered = Image2D::Create(imageSpec);

			for (uint32_t i = 0; i < imageSpec.mips; i++)
			{
				environmentFiltered->CreateMipView(i);
			}

			{
				VkCommandBuffer cmdBuffer = device->GetCommandBuffer(true);
				environmentFiltered->TransitionToLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);
				device->FlushCommandBuffer(cmdBuffer);
			}

			Ref<ComputePipeline> filterPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("EnvironmentMipFilter"), 1, true);

			const float deltaRoughness = 1.f / glm::max((float)imageSpec.mips - 1.f, 1.f);
			for (uint32_t i = 0, size = cubeMapSize; i < imageSpec.mips; i++, size /= 2)
			{
				const uint32_t numGroups = glm::max(1u, size / 32);

				float roughness = i * deltaRoughness;
				roughness = glm::max(roughness, 0.05f);

				VkCommandBuffer cmdBuffer = device->GetCommandBuffer(true);

				filterPipeline->Clear(0);
				filterPipeline->Bind(cmdBuffer);
				filterPipeline->BindDescriptorSet(cmdBuffer, GetBindlessData().globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(0), Sets::SAMPLERS);

				filterPipeline->SetImage(environmentFiltered, Sets::OTHER, 0, i, ImageAccess::Write);
				filterPipeline->SetImage(environmentUnfiltered, Sets::OTHER, 1, ImageAccess::Write);

				filterPipeline->PushConstants(cmdBuffer, &roughness, sizeof(float));

				ImageBarrierInfo barrierInfo{};
				barrierInfo.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
				barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				barrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
				barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

				barrierInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrierInfo.subresourceRange.baseArrayLayer = 0;
				barrierInfo.subresourceRange.baseMipLevel = i;
				barrierInfo.subresourceRange.layerCount = imageSpec.layers;
				barrierInfo.subresourceRange.levelCount = 1;

				filterPipeline->InsertImageBarrier(Sets::OTHER, 0, barrierInfo);
				filterPipeline->Dispatch(cmdBuffer, numGroups, numGroups, 6);
				filterPipeline->InsertBarriers(cmdBuffer);

				device->FlushCommandBuffer(cmdBuffer);
			}
		}

		// Irradiance
		{
			ImageSpecification imageSpec{};
			imageSpec.format = ImageFormat::RGBA32F;
			imageSpec.width = irradianceMapSize;
			imageSpec.height = irradianceMapSize;
			imageSpec.usage = ImageUsage::Storage;
			imageSpec.layers = 6;
			imageSpec.isCubeMap = true;
			imageSpec.mips = Utility::CalculateMipCount(irradianceMapSize, irradianceMapSize);

			irradianceMap = Image2D::Create(imageSpec);

			Ref<ComputePipeline> irradiancePipeline = ComputePipeline::Create(ShaderRegistry::GetShader("EnvironmentIrradiance"), 1, true);

			VkCommandBuffer cmdBuffer = device->GetCommandBuffer(true);
			environmentFiltered->TransitionToLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			irradianceMap->TransitionToLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);

			irradiancePipeline->Bind(cmdBuffer);
			irradiancePipeline->BindDescriptorSet(cmdBuffer, GetBindlessData().globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(0), Sets::SAMPLERS);

			irradiancePipeline->SetImage(irradianceMap, Sets::OTHER, 0, ImageAccess::Write);
			irradiancePipeline->SetImage(environmentFiltered, Sets::OTHER, 1, ImageAccess::Read);

			ExecutionBarrierInfo barrierInfo{};
			barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
			barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.dstAccessMask = 0;
			barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

			irradiancePipeline->InsertExecutionBarrier(barrierInfo);
			irradiancePipeline->Dispatch(cmdBuffer, irradianceMapSize / conversionThreadCount, irradianceMapSize / conversionThreadCount, 6);
			irradiancePipeline->InsertBarriers(cmdBuffer);

			irradianceMap->GenerateMips(false, cmdBuffer);
			irradianceMap->TransitionToLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			device->FlushCommandBuffer(cmdBuffer);
		}

		return { irradianceMap, environmentFiltered };
	}

	const uint32_t Renderer::AddTexture(Ref<Image2D> image)
	{
		std::scoped_lock lock{ s_textureMutex };
		return s_rendererData->bindlessData.textureTable->AddTexture(image);
	}

	void Renderer::RemoveTexture(Ref<Image2D> image)
	{
		std::scoped_lock lock{ s_textureMutex };
		s_rendererData->bindlessData.textureTable->RemoveTexture(image);
	}

	void Renderer::ReloadShader(Ref<Shader> shader)
	{
		if (!s_shaderDependencies.contains(shader->GetHash()))
		{
			return;
		}

		auto& dependencies = s_shaderDependencies.at(shader->GetHash());

		for (const auto& renderPipeline : dependencies.renderPipelines)
		{
			renderPipeline->Invalidate();
		}

		for (const auto& computePipeline : dependencies.computePipelines)
		{
			computePipeline->Invalidate();
		}

		for (const auto& material : dependencies.materials)
		{
			material->Invalidate();
		}

		for (const auto& postMat : dependencies.postProcessingMaterials)
		{
			postMat->Invalidate();
		}
	}

	void Renderer::AddShaderDependency(Ref<Shader> shader, RenderPipeline* renderPipeline)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };
		s_shaderDependencies[shader->GetHash()].renderPipelines.emplace_back(renderPipeline);
	}

	void Renderer::AddShaderDependency(Ref<Shader> shader, ComputePipeline* computePipeline)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };
		s_shaderDependencies[shader->GetHash()].computePipelines.emplace_back(computePipeline);
	}

	void Renderer::AddShaderDependency(Ref<Shader> shader, SubMaterial* subMaterial)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };
		s_shaderDependencies[shader->GetHash()].materials.emplace_back(subMaterial);
	}

	void Renderer::AddShaderDependency(Ref<Shader> shader, PostProcessingMaterial* postMat)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };
		s_shaderDependencies[shader->GetHash()].postProcessingMaterials.emplace_back(postMat);
	}

	void Renderer::RemoveShaderDependency(Ref<Shader> shader, RenderPipeline* renderPipeline)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };

		if (!s_shaderDependencies.contains(shader->GetHash()))
		{
			VT_CORE_WARN("[Renderer] Trying to remove render pipeline dependency from shader with hash {0}, but the shader is not valid!", shader->GetHash());
			return;
		}

		auto& renderPipelines = s_shaderDependencies.at(shader->GetHash()).renderPipelines;
		if (auto it = std::find(renderPipelines.begin(), renderPipelines.end(), renderPipeline); it != renderPipelines.end())
		{
			renderPipelines.erase(it);
		}
	}

	void Renderer::RemoveShaderDependency(Ref<Shader> shader, ComputePipeline* computePipeline)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };

		if (!s_shaderDependencies.contains(shader->GetHash()))
		{
			VT_CORE_WARN("[Renderer] Trying to remove compute pipeline dependency from shader with hash {0}, but the shader is not valid!", shader->GetHash());
			return;
		}

		auto& computePipelines = s_shaderDependencies.at(shader->GetHash()).computePipelines;
		if (auto it = std::find(computePipelines.begin(), computePipelines.end(), computePipeline); it != computePipelines.end())
		{
			computePipelines.erase(it);
		}
	}

	void Renderer::RemoveShaderDependency(Ref<Shader> shader, SubMaterial* material)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };

		if (!s_shaderDependencies.contains(shader->GetHash()))
		{
			VT_CORE_WARN("[Renderer] Trying to remove material dependency from shader with hash {0}, but the shader is not valid!", shader->GetHash());
			return;
		}

		auto& materials = s_shaderDependencies.at(shader->GetHash()).materials;
		if (auto it = std::find(materials.begin(), materials.end(), material); it != materials.end())
		{
			materials.erase(it);
		}
	}

	void Renderer::RemoveShaderDependency(Ref<Shader> shader, PostProcessingMaterial* postMat)
	{
		std::scoped_lock lock{ s_shaderDependenciesMutex };

		if (!s_shaderDependencies.contains(shader->GetHash()))
		{
			VT_CORE_WARN("[Renderer] Trying to remove material dependency from shader with hash {0}, but the shader is not valid!", shader->GetHash());
			return;
		}

		auto& materials = s_shaderDependencies.at(shader->GetHash()).postProcessingMaterials;
		if (auto it = std::find(materials.begin(), materials.end(), postMat); it != materials.end())
		{
			materials.erase(it);
		}
	}

	const uint32_t Renderer::AddMaterial(SubMaterial* material)
	{
		std::scoped_lock lock{ s_materialMutex };
		return s_rendererData->bindlessData.materialTable->AddMaterial(material);
	}

	void Renderer::RemoveMaterial(SubMaterial* material)
	{
		std::scoped_lock lock{ s_materialMutex };

		if (s_rendererData)
		{
			s_rendererData->bindlessData.materialTable->RemoveMaterial(material);
		}
	}

	void Renderer::UpdateMaterial(SubMaterial* material)
	{
		std::scoped_lock lock{ s_materialMutex };
		s_rendererData->bindlessData.materialTable->UpdateMaterial(material);
	}

	const uint32_t Renderer::AddMesh(Mesh* mesh, const uint32_t subMeshIndex)
	{
		std::scoped_lock lock{ s_meshMutex };
		return s_rendererData->bindlessData.meshTable->AddMesh(mesh, subMeshIndex);
	}

	void Renderer::RemoveMesh(Mesh* mesh, const uint32_t subMeshIndex)
	{
		if (!s_rendererData)
		{
			return;
		}

		std::scoped_lock lock{ s_meshMutex };
		s_rendererData->bindlessData.meshTable->RemoveMesh(mesh, subMeshIndex);
	}

	Renderer::BindlessData& Renderer::GetBindlessData()
	{
		return s_rendererData->bindlessData;
	}

	Renderer::DefaultData& Renderer::GetDefaultData()
	{
		return *s_defaultData;
	}

	const VulkanFunctions& Renderer::GetVulkanFunctions()
	{
		return s_rendererData->vulkanFunctions;
	}

	const uint32_t Renderer::GetFramesInFlightCount()
	{
		return s_rendererData->framesInFlightCount;
	}

	VkDescriptorPool Renderer::GetCurrentDescriptorPool()
	{
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();
		return s_rendererData->perFrameDescriptorPools.at(currentFrame);
	}

	VkDescriptorPool Renderer::GetDescriptorPool(uint32_t index)
	{
		return s_rendererData->perFrameDescriptorPools.at(index);
	}

	VkDescriptorSet Renderer::AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		VT_PROFILE_FUNCTION();
		auto device = GraphicsContext::GetDevice();
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

		allocInfo.descriptorPool = s_rendererData->perFrameDescriptorPools.at(currentFrame);

		VkDescriptorSet descriptorSet{};
		VT_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &descriptorSet));

		return descriptorSet;
	}

	VkDescriptorSet Renderer::AllocatePersistentDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		VT_PROFILE_FUNCTION();
		auto device = GraphicsContext::GetDevice();
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

		allocInfo.descriptorPool = s_rendererData->perFramePersistentDescriptorPools.at(currentFrame);

		VkDescriptorSet descriptorSet{};
		VT_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &descriptorSet));

		return descriptorSet;
	}

	void Renderer::CreateBindlessData()
	{
		auto& data = GetBindlessData();
		//data.combinedVertexBuffer = CombinedVertexBuffer::Create(sizeof(Vertex), BindlessData::START_VERTEX_COUNT);
		//data.combinedIndexBuffer = CombinedIndexBuffer::Create(BindlessData::START_INDEX_COUNT);
		data.textureTable = TextureTable::Create();
		data.materialTable = MaterialTable::Create();
		data.meshTable = MeshTable::Create();
		data.samplerStateSet = SamplerStateSet::Create(GetFramesInFlightCount());

		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_LINEAR, TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::None);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_LINEAR_POINT, TextureFilter::Linear, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Repeat, CompareOperator::None);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_LINEAR_CLAMP, TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Clamp, CompareOperator::None);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_LINEAR_POINT_CLAMP, TextureFilter::Linear, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Clamp, CompareOperator::None);

		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_POINT, TextureFilter::Nearest, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Repeat, CompareOperator::None);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_POINT_LINEAR, TextureFilter::Nearest, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::None);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_POINT_CLAMP, TextureFilter::Nearest, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Clamp, CompareOperator::None);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_POINT_LINEAR_CLAMP, TextureFilter::Nearest, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Clamp, CompareOperator::None);

		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_ANISO, TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::None, AnisotopyLevel::X16);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_SHADOW, TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::LessEqual);
		data.samplerStateSet->Add(Sets::SAMPLERS, Bindings::SAMPLER_REDUCE, TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::None, AnisotopyLevel::None, true);
	}

	void Renderer::CreateDescriptorPools()
	{
		constexpr VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 10000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = 0;
		poolInfo.maxSets = 10000;
		poolInfo.poolSizeCount = (uint32_t)ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		const uint32_t framesInFlightCount = GetFramesInFlightCount();
		for (uint32_t i = 0; i < framesInFlightCount; i++)
		{
			VT_VK_CHECK(vkCreateDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), &poolInfo, nullptr, &s_rendererData->perFrameDescriptorPools.emplace_back()));
		}

		for (uint32_t i = 0; i < framesInFlightCount; i++)
		{
			VT_VK_CHECK(vkCreateDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), &poolInfo, nullptr, &s_rendererData->perFramePersistentDescriptorPools.emplace_back()));
		}
	}

	void Renderer::CreateGlobalDescriptorSets()
	{
		auto& bindlessData = GetBindlessData();
		bindlessData.globalDescriptorSets = GlobalDescriptorSetManager::CreateFullCopy();
	}

	void Renderer::CreateDefaultData()
	{
		s_defaultData = CreateScope<DefaultData>();
		s_defaultData->defaultShader = Shader::Create("Default", { "Engine/Shaders/Source/HLSL/Default_vs.hlsl", "Engine/Shaders/Source/HLSL/Default_ps.hlsl" });
		s_defaultData->defaultPostProcessingShader = Shader::Create("DefaultPostProcessing", { "Engine/Shaders/Source/HLSL/DefaultPostProcessing_cs.hlsl" });

		// Create default render pipeline
		{
			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.shader = s_defaultData->defaultShader;

			pipelineSpec.vertexLayout = Vertex::GetVertexLayout();
			s_defaultData->defaultRenderPipeline = RenderPipeline::Create(pipelineSpec);

			ShaderRegistry::Register("Default", s_defaultData->defaultShader);
		}

		s_defaultData->defaultMaterial = SubMaterial::Create("Default", 0, s_defaultData->defaultShader);

		// White texture
		{
			constexpr uint32_t whiteTextureData = 0xffffffff;
			s_defaultData->whiteTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1, &whiteTextureData);
			s_defaultData->whiteTexture->handle = Asset::Null();
		}

		// White uint texture
		{
			constexpr uint32_t whiteTextureData = 0xffffffff;
			s_defaultData->whiteUINTTexture = Texture2D::Create(ImageFormat::R32UI, 1, 1, &whiteTextureData);
			s_defaultData->whiteUINTTexture->handle = Asset::Null();
		}

		// Empty normal
		{
			const glm::vec4 normalData = { 0.f, 0.5f, 1.f, 0.5f };
			s_defaultData->emptyNormal = Texture2D::Create(ImageFormat::RGBA32F, 1, 1, &normalData);
			s_defaultData->emptyNormal->handle = Asset::Null();
		}

		// Empty Material
		{
			const glm::vec4 materialData = { 0.f, 1.f, 1.f, 0.f };
			s_defaultData->emptyMaterial = Texture2D::Create(ImageFormat::RGBA32F, 1, 1, &materialData);
			s_defaultData->emptyMaterial->handle = Asset::Null();
		}

		// Black 3D
		{
			ImageSpecification spec{};
			spec.format = ImageFormat::RGBA32F;
			spec.width = 1;
			spec.height = 1;
			spec.depth = 1;
			spec.usage = ImageUsage::Texture;
			spec.debugName = "Black 3D Image";

			const glm::vec4 blackColor = { 0.f };
			s_defaultData->black3DImage = Image3D::Create(spec, &blackColor);
		}
	}

	void Renderer::GenerateBRDFLut()
	{
		constexpr uint32_t BRDFSize = 512;

		ImageSpecification spec{};
		spec.format = ImageFormat::RG16F;
		spec.usage = ImageUsage::Storage;
		spec.width = BRDFSize;
		spec.height = BRDFSize;

		s_defaultData->brdfLut = Image2D::Create(spec);

		Ref<ComputePipeline> brdfPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("BRDFGeneration"));
		brdfPipeline->SetImage(s_defaultData->brdfLut, 0, 0, ImageAccess::Write);
		brdfPipeline->Execute(BRDFSize / 32, BRDFSize / 32, 1);
	}

	void Renderer::LoadVulkanFunctions()
	{
		auto instance = GraphicsContext::Get().GetInstance();

		s_rendererData->vulkanFunctions.cmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
		s_rendererData->vulkanFunctions.cmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
	}

	void Renderer::LoadRayTracingFunctions()
	{
		auto device = GraphicsContext::GetDevice()->GetHandle();

		s_rendererData->vulkanFunctions.vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
		s_rendererData->vulkanFunctions.vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
		s_rendererData->vulkanFunctions.vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
		s_rendererData->vulkanFunctions.vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
		s_rendererData->vulkanFunctions.vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
		s_rendererData->vulkanFunctions.vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
		s_rendererData->vulkanFunctions.vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
		s_rendererData->vulkanFunctions.vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
		s_rendererData->vulkanFunctions.vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
	}

	void Renderer::DestroyDescriptorPools()
	{
		auto device = GraphicsContext::GetDevice();
		device->WaitForIdle();

		for (const auto& pool : s_rendererData->perFrameDescriptorPools)
		{
			vkDestroyDescriptorPool(device->GetHandle(), pool, nullptr);
		}

		for (const auto& pool : s_rendererData->perFramePersistentDescriptorPools)
		{
			vkDestroyDescriptorPool(device->GetHandle(), pool, nullptr);
		}

		s_rendererData->perFrameDescriptorPools.clear();
	}

	void Renderer::UpdateMaterialDescriptors()
	{
		VT_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < GetFramesInFlightCount(); i++)
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::MAINBUFFERS]->GetOrAllocateDescriptorSet(i);
			const auto buffer = s_rendererData->bindlessData.materialTable->GetStorageBuffer(i);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer->GetHandle();
			bufferInfo.offset = 0;
			bufferInfo.range = buffer->GetSize();

			VkWriteDescriptorSet writeDescriptor{};
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.dstSet = descriptorSet;
			writeDescriptor.dstBinding = Bindings::MATERIAL_TABLE;
			writeDescriptor.dstArrayElement = 0;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptor.pBufferInfo = &bufferInfo;

			auto device = GraphicsContext::GetDevice();
			vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
		}
	}

	void Renderer::UpdateMeshDescriptors()
	{
		VT_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < GetFramesInFlightCount(); i++)
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::MAINBUFFERS]->GetOrAllocateDescriptorSet(i);
			const auto buffer = s_rendererData->bindlessData.meshTable->GetStorageBuffer(i);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer->GetHandle();
			bufferInfo.offset = 0;
			bufferInfo.range = buffer->GetSize();

			VkWriteDescriptorSet writeDescriptor{};
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.dstSet = descriptorSet;
			writeDescriptor.dstBinding = Bindings::MESH_TABLE;
			writeDescriptor.dstArrayElement = 0;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptor.pBufferInfo = &bufferInfo;

			auto device = GraphicsContext::GetDevice();
			vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
		}
	}

	void Renderer::UpdateSamplerStateDescriptors()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();

		for (uint32_t i = 0; i < GetFramesInFlightCount(); i++)
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(i);
			const auto writeDescriptors = s_rendererData->bindlessData.samplerStateSet->GetWriteDescriptors(descriptorSet, i);
			if (writeDescriptors.empty())
			{
				return;
			}

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		}
	}

	void Renderer::SetupPipelineForRendering(Ref<CommandBuffer> commandBuffer, Ref<RenderPipeline> renderPipeline, const GlobalDescriptorMap& globalDescriptorSets, Ref<GlobalDescriptorSet> drawBufferSet)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		if (renderPipeline->HasDescriptorSet(Sets::RENDERER_BUFFERS))
		{
			auto descriptorSet = globalDescriptorSets.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(currentIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::RENDERER_BUFFERS);
		}

		if (renderPipeline->HasDescriptorSet(Sets::SAMPLERS))
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(currentIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::SAMPLERS);
		}

		if (renderPipeline->HasDescriptorSet(Sets::TEXTURES))
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::TEXTURES]->GetOrAllocateDescriptorSet(currentIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::TEXTURES);
		}

		if (renderPipeline->HasDescriptorSet(Sets::MAINBUFFERS))
		{
			const auto appIndex = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::MAINBUFFERS]->GetOrAllocateDescriptorSet(appIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::MAINBUFFERS);
		}

		if (renderPipeline->HasDescriptorSet(Sets::PBR_RESOURCES))
		{
			auto descriptorSet = globalDescriptorSets.at(Sets::PBR_RESOURCES)->GetOrAllocateDescriptorSet(currentIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::PBR_RESOURCES);
		}

		if (drawBufferSet && renderPipeline->HasDescriptorSet(Sets::DRAW_BUFFERS))
		{
			auto descriptorSet = drawBufferSet->GetOrAllocateDescriptorSet(currentIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::DRAW_BUFFERS);
		}
	}

	void Renderer::SetupPipelineForRendering(Ref<CommandBuffer> commandBuffer, Ref<RenderPipeline> renderPipeline)
	{
		VT_PROFILE_FUNCTION();

		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		if (renderPipeline->HasDescriptorSet(Sets::SAMPLERS))
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(currentIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::SAMPLERS);
		}

		if (renderPipeline->HasDescriptorSet(Sets::TEXTURES))
		{
			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::TEXTURES]->GetOrAllocateDescriptorSet(currentIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::TEXTURES);
		}

		if (renderPipeline->HasDescriptorSet(Sets::MAINBUFFERS))
		{
			const auto appIndex = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

			auto descriptorSet = s_rendererData->bindlessData.globalDescriptorSets[Sets::MAINBUFFERS]->GetOrAllocateDescriptorSet(appIndex);
			renderPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), descriptorSet, Sets::MAINBUFFERS);
		}
	}
}
