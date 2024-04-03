#include "vtpch.h"
#include "RendererNew.h"

#include "Volt/Core/Application.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Utility/FunctionQueue.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"
#include "Volt/RenderingNew/Shader/ShaderMap.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"
#include "Volt/RenderingNew/Debug/ShaderRuntimeValidator.h"
#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/Math/Math.h"

#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Graphics/Swapchain.h>
#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

namespace Volt
{
	namespace Utility
	{
		inline static const size_t GetHashFromSamplerInfo(const RHI::SamplerStateCreateInfo& info)
		{
			size_t hash = std::hash<uint32_t>()(static_cast<uint32_t>(info.minFilter));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(info.magFilter)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(info.mipFilter)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(info.wrapMode)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(info.compareOperator)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(info.anisotropyLevel)));
			hash = Math::HashCombine(hash, std::hash<float>()(info.mipLodBias));
			hash = Math::HashCombine(hash, std::hash<float>()(info.minLod));
			hash = Math::HashCombine(hash, std::hash<float>()(info.maxLod));

			return hash;
		}
	}

	struct RendererData
	{
		Ref<RHI::ShaderCompiler> shaderCompiler;

#ifndef VT_DIST
		Scope<ShaderRuntimeValidator> shaderValidator;
#endif

		std::vector<FunctionQueue> deletionQueue;

		std::unordered_map<size_t, Ref<GlobalResource<RHI::SamplerState>>> samplers;

		DefaultResources defaultResources;

		inline void Shutdown()
		{
			samplers.clear();
			shaderCompiler = nullptr;

			for (auto& resourceQueue : deletionQueue)
			{
				resourceQueue.Flush();
			}
		}
	};

	Scope<RendererData> s_rendererData;

	void RendererNew::PreInitialize()
	{
		s_rendererData = CreateScope<RendererData>();
		s_rendererData->deletionQueue.resize(Application::Get().GetWindow().GetSwapchain().GetFramesInFlight());

		// Create shader compiler
		{
			RHI::ShaderCompilerCreateInfo shaderCompilerInfo{};
			shaderCompilerInfo.flags = RHI::ShaderCompilerFlags::WarningsAsErrors;

#ifndef VT_DIST
			shaderCompilerInfo.flags |= RHI::ShaderCompilerFlags::EnableShaderValidator;
#endif

			//shaderCompilerInfo.cacheDirectory = ProjectManager::GetEngineDirectory() / "Engine/Shaders/Cache";
			shaderCompilerInfo.includeDirectories =
			{
				"Engine/Shaders/Source/Includes",
				"Engine/Shaders/Source/HLSL",
				"Engine/Shaders/Source/HLSL/Includes",
				ProjectManager::GetAssetsDirectory()
			};

			s_rendererData->shaderCompiler = RHI::ShaderCompiler::Create(shaderCompilerInfo);
		}
	}

	void RendererNew::Initialize()
	{
		GlobalResourceManager::Initialize();
		RenderGraphExecutionThread::Initialize();

		CreateDefaultResources();

#ifndef VT_DIST
		s_rendererData->shaderValidator = CreateScope<ShaderRuntimeValidator>();
#endif
	}

	void RendererNew::Shutdown()
	{
		RenderGraphExecutionThread::Shutdown();
		GlobalResourceManager::Shutdown();

		s_rendererData->Shutdown();
		s_rendererData = nullptr;
	}

	void RendererNew::Flush()
	{
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();

		//Application::GetThreadPool().SubmitTask([currentFrame, queueCopy = s_rendererData->deletionQueue.at(currentFrame)]() mutable
		//{
		//	queueCopy.Flush();
		//});

		s_rendererData->deletionQueue.at(currentFrame).Flush();
	}

	const uint32_t RendererNew::GetFramesInFlight()
	{
		return Application::Get().GetWindow().GetSwapchain().GetFramesInFlight();
	}

	void RendererNew::DestroyResource(std::function<void()>&& function)
	{
		if (!s_rendererData)
		{
			function();
			return;
		}

		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();
		s_rendererData->deletionQueue.at(currentFrame).Push(std::move(function));
	}

	const DefaultResources& RendererNew::GetDefaultResources()
	{
		return s_rendererData->defaultResources;
	}

	SceneEnvironment RendererNew::GenerateEnvironmentTextures(AssetHandle baseTextureHandle)
	{
		Ref<Texture2D> environmentTexture = AssetManager::GetAsset<Texture2D>(baseTextureHandle);
		if (!environmentTexture || !environmentTexture->IsValid())
		{
			return {};
		}

		constexpr uint32_t CUBE_MAP_SIZE = 1024;
		constexpr uint32_t IRRADIANCE_MAP_SIZE = 32;
		constexpr uint32_t CONVERSION_THREAD_GROUP_SIZE = 32;

		Ref<RHI::Image2D> environmentUnfiltered;
		Ref<RHI::Image2D> environmentFiltered;
		Ref<RHI::Image2D> irradianceMap;

		auto linearSampler = GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>();

		Ref<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
		commandBuffer->Begin();

		// Unfiltered - Conversion
		{
			RHI::ImageSpecification imageSpec{};
			imageSpec.format = RHI::PixelFormat::B10G11R11_UFLOAT_PACK32;
			imageSpec.width = CUBE_MAP_SIZE;
			imageSpec.height = CUBE_MAP_SIZE;
			imageSpec.usage = RHI::ImageUsage::Storage;
			imageSpec.layers = 6;
			imageSpec.isCubeMap = true;

			environmentUnfiltered = RHI::Image2D::Create(imageSpec);

			{
				RHI::ResourceBarrierInfo barrierInfo{};
				barrierInfo.type = RHI::BarrierType::Image;
				barrierInfo.imageBarrier().srcStage = RHI::BarrierStage::None;
				barrierInfo.imageBarrier().srcAccess = RHI::BarrierAccess::None;
				barrierInfo.imageBarrier().srcLayout = RHI::ImageLayout::Undefined;
				barrierInfo.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader;
				barrierInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderWrite;
				barrierInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderWrite;
				barrierInfo.imageBarrier().resource = environmentUnfiltered;
				commandBuffer->ResourceBarrier({ barrierInfo });
			}

			auto conversionPipeline = ShaderMap::GetComputePipeline("EquirectangularToCubemap", false);

			RHI::DescriptorTableCreateInfo tableInfo{};
			tableInfo.shader = conversionPipeline->GetShader();

			Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(tableInfo);
			descriptorTable->SetImageView("o_output", environmentUnfiltered->GetArrayView(), 0);
			descriptorTable->SetImageView("u_equirectangularMap", environmentTexture->GetImage()->GetView(), 0);
			descriptorTable->SetSamplerState("u_linearSampler", linearSampler->GetResource(), 0);

			commandBuffer->BindPipeline(conversionPipeline);
			commandBuffer->BindDescriptorTable(descriptorTable);

			const uint32_t groupCount = Math::DivideRoundUp(CUBE_MAP_SIZE, CONVERSION_THREAD_GROUP_SIZE);
			commandBuffer->Dispatch(groupCount, groupCount, 6);

			{
				RHI::ResourceBarrierInfo imageBarrierInfo{};
				imageBarrierInfo.type = RHI::BarrierType::Image;
				imageBarrierInfo.imageBarrier().srcStage = RHI::BarrierStage::ComputeShader;
				imageBarrierInfo.imageBarrier().srcAccess = RHI::BarrierAccess::ShaderWrite;
				imageBarrierInfo.imageBarrier().srcLayout = RHI::ImageLayout::ShaderWrite;
				imageBarrierInfo.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader;
				imageBarrierInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
				imageBarrierInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
				imageBarrierInfo.imageBarrier().resource = environmentUnfiltered;

				RHI::ResourceBarrierInfo barrierInfo{};
				barrierInfo.type = RHI::BarrierType::Global;
				barrierInfo.globalBarrier().srcAccess = RHI::BarrierAccess::ShaderWrite;
				barrierInfo.globalBarrier().srcStage = RHI::BarrierStage::ComputeShader;
				barrierInfo.globalBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
				barrierInfo.globalBarrier().dstStage = RHI::BarrierStage::ComputeShader;
				commandBuffer->ResourceBarrier({ barrierInfo, imageBarrierInfo });
			}
		}

		// Filtered
		{
			RHI::ImageSpecification imageSpec{};
			imageSpec.format = RHI::PixelFormat::B10G11R11_UFLOAT_PACK32;
			imageSpec.width = CUBE_MAP_SIZE;
			imageSpec.height = CUBE_MAP_SIZE;
			imageSpec.usage = RHI::ImageUsage::Storage;
			imageSpec.layers = 6;
			imageSpec.isCubeMap = true;
			imageSpec.mips = RHI::Utility::CalculateMipCount(CUBE_MAP_SIZE, CUBE_MAP_SIZE);

			environmentFiltered = RHI::Image2D::Create(imageSpec);

			for (uint32_t i = 0; i < imageSpec.mips; i++)
			{
				environmentFiltered->GetArrayView(i);
			}

			{
				RHI::ResourceBarrierInfo barrierInfo{};
				barrierInfo.type = RHI::BarrierType::Image;
				barrierInfo.imageBarrier().srcStage = RHI::BarrierStage::None;
				barrierInfo.imageBarrier().srcAccess = RHI::BarrierAccess::None;
				barrierInfo.imageBarrier().srcLayout = RHI::ImageLayout::Undefined;
				barrierInfo.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader;
				barrierInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderWrite;
				barrierInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderWrite;
				barrierInfo.imageBarrier().resource = environmentFiltered;
				commandBuffer->ResourceBarrier({ barrierInfo });
			}

			auto pipeline = ShaderMap::GetComputePipeline("EnvironmentMipFilter", false);
			RHI::DescriptorTableCreateInfo tableInfo{};
			tableInfo.shader = pipeline->GetShader();
			tableInfo.count = imageSpec.mips;

			Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(tableInfo);
			descriptorTable->SetImageView("u_input", environmentUnfiltered->GetView(), 0);
			descriptorTable->SetSamplerState("u_linearSampler", linearSampler->GetResource(), 0);

			struct Constants
			{
				float roughness;
			} constants;

			const float deltaRoughness = 1.f / glm::max(static_cast<float>(imageSpec.mips) - 1.f, 1.f);
			for (uint32_t i = 0, size = CUBE_MAP_SIZE; i < imageSpec.mips; i++, size /= 2)
			{
				const uint32_t numGroups = glm::max(1u, Math::DivideRoundUp(size, 32u));

				float roughness = i * deltaRoughness;
				roughness = glm::max(roughness, 0.05f);

				constants.roughness = roughness;

				descriptorTable->SetImageView("o_output", environmentFiltered->GetArrayView(i), 0);

				commandBuffer->BindPipeline(pipeline);
				commandBuffer->BindDescriptorTable(descriptorTable);
				commandBuffer->PushConstants(&constants, sizeof(Constants), 0);
				commandBuffer->Dispatch(numGroups, numGroups, 6);

				RHI::ResourceBarrierInfo imageBarrierInfo{};
				imageBarrierInfo.type = RHI::BarrierType::Image;
				imageBarrierInfo.imageBarrier().srcStage = RHI::BarrierStage::ComputeShader;
				imageBarrierInfo.imageBarrier().srcAccess = RHI::BarrierAccess::ShaderWrite;
				imageBarrierInfo.imageBarrier().srcLayout = RHI::ImageLayout::ShaderWrite;
				imageBarrierInfo.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader;
				imageBarrierInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
				imageBarrierInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
				imageBarrierInfo.imageBarrier().subResource.levelCount = 1;
				imageBarrierInfo.imageBarrier().subResource.baseMipLevel = i;
				imageBarrierInfo.imageBarrier().resource = environmentFiltered;

				commandBuffer->ResourceBarrier({ imageBarrierInfo });
			}
		}

		// Irradiance
		{
			RHI::ImageSpecification imageSpec{};
			imageSpec.format = RHI::PixelFormat::B10G11R11_UFLOAT_PACK32;
			imageSpec.width = IRRADIANCE_MAP_SIZE;
			imageSpec.height = IRRADIANCE_MAP_SIZE;
			imageSpec.usage = RHI::ImageUsage::Storage;
			imageSpec.layers = 6;
			imageSpec.isCubeMap = true;
			imageSpec.mips = RHI::Utility::CalculateMipCount(IRRADIANCE_MAP_SIZE, IRRADIANCE_MAP_SIZE);

			irradianceMap = RHI::Image2D::Create(imageSpec);
		
			{
				RHI::ResourceBarrierInfo barrierInfo{};
				barrierInfo.type = RHI::BarrierType::Image;
				barrierInfo.imageBarrier().srcStage = RHI::BarrierStage::None;
				barrierInfo.imageBarrier().srcAccess = RHI::BarrierAccess::None;
				barrierInfo.imageBarrier().srcLayout = RHI::ImageLayout::Undefined;
				barrierInfo.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader;
				barrierInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderWrite;
				barrierInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderWrite;
				barrierInfo.imageBarrier().resource = irradianceMap;
				commandBuffer->ResourceBarrier({ barrierInfo });
			}

			auto pipeline = ShaderMap::GetComputePipeline("EnvironmentIrradiance", false);
			RHI::DescriptorTableCreateInfo tableInfo{};
			tableInfo.shader = pipeline->GetShader();

			Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(tableInfo);
			descriptorTable->SetImageView("o_output", irradianceMap->GetArrayView(), 0);
			descriptorTable->SetImageView("u_input", environmentFiltered->GetView(), 0);
			descriptorTable->SetSamplerState("u_linearSampler", linearSampler->GetResource(), 0);

			commandBuffer->BindPipeline(pipeline);
			commandBuffer->BindDescriptorTable(descriptorTable);

			const uint32_t groupCount = Math::DivideRoundUp(IRRADIANCE_MAP_SIZE, CONVERSION_THREAD_GROUP_SIZE);
			commandBuffer->Dispatch(groupCount, groupCount, 6);

			{
				RHI::ResourceBarrierInfo imageBarrierInfo{};
				imageBarrierInfo.type = RHI::BarrierType::Image;
				imageBarrierInfo.imageBarrier().srcStage = RHI::BarrierStage::ComputeShader;
				imageBarrierInfo.imageBarrier().srcAccess = RHI::BarrierAccess::ShaderWrite;
				imageBarrierInfo.imageBarrier().srcLayout = RHI::ImageLayout::ShaderWrite;
				imageBarrierInfo.imageBarrier().dstStage = RHI::BarrierStage::PixelShader | RHI::BarrierStage::ComputeShader;
				imageBarrierInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
				imageBarrierInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
				imageBarrierInfo.imageBarrier().resource = irradianceMap;

				commandBuffer->ResourceBarrier({ imageBarrierInfo });
			}

		}

		commandBuffer->End();
		commandBuffer->ExecuteAndWait();

		irradianceMap->GenerateMips();

		SceneEnvironment result{};
		result.irradianceMap = irradianceMap;
		result.radianceMap = environmentFiltered;

		return result;
	}

#ifndef VT_DIST
	ShaderRuntimeValidator& RendererNew::GetRuntimeShaderValidator()
	{
		return *s_rendererData->shaderValidator;
	}
#endif

	void RendererNew::Update()
	{
		//GlobalResourceManager::Update();
	}

	void RendererNew::EndOfFrameUpdate()
	{
#ifndef VT_DIST
		s_rendererData->shaderValidator->Update();

		const auto& frameErrors = s_rendererData->shaderValidator->GetValidationErrors();
		for (const auto& error : frameErrors)
		{
			VT_CORE_ERROR(error);
		}
#endif
	}

	Ref<GlobalResource<RHI::SamplerState>> RendererNew::GetSamplerInternal(const RHI::SamplerStateCreateInfo& samplerInfo)
	{
		const size_t hash = Utility::GetHashFromSamplerInfo(samplerInfo);
		if (s_rendererData->samplers.contains(hash))
		{
			return s_rendererData->samplers.at(hash);
		}

		Ref<GlobalResource<RHI::SamplerState>> samplerState = GlobalResource<RHI::SamplerState>::Create(RHI::SamplerState::Create(samplerInfo));
		s_rendererData->samplers[hash] = samplerState;

		return samplerState;
	}

	void RendererNew::CreateDefaultResources()
	{
		// Full white 1x1
		{
			constexpr uint32_t PIXEL_DATA = 0xffffffff;
			s_rendererData->defaultResources.whiteTexture = Texture2D::Create(RHI::PixelFormat::R8G8B8A8_UNORM, 1, 1, &PIXEL_DATA);
			s_rendererData->defaultResources.whiteTexture->handle = 0;
		}

		// Full black cube 1x1
		{
			constexpr uint32_t PIXEL_DATA[6] = { 0, 0, 0, 0, 0, 0 };

			RHI::ImageSpecification imageSpec{};
			imageSpec.format = RHI::PixelFormat::B10G11R11_UFLOAT_PACK32;
			imageSpec.usage = RHI::ImageUsage::Texture;
			imageSpec.width = 1;
			imageSpec.height = 1;
			imageSpec.layers = 6;
			imageSpec.isCubeMap = true;

			s_rendererData->defaultResources.blackCubeTexture = RHI::Image2D::Create(imageSpec, PIXEL_DATA);
		}

		GenerateBRDFLuT();

		// Default material
		{
			s_rendererData->defaultResources.defaultMaterial = AssetManager::CreateMemoryAsset<Material>("DefaultMaterial", ShaderMap::GetComputePipeline("OpaqueDefault"));
		}
	}
	
	void RendererNew::GenerateBRDFLuT()
	{
		constexpr uint32_t BRDFSize = 512;

		RHI::ImageSpecification spec{};
		spec.format = RHI::PixelFormat::R16G16_SFLOAT;
		spec.usage = RHI::ImageUsage::Storage;
		spec.width = BRDFSize;
		spec.height = BRDFSize;

		s_rendererData->defaultResources.BRDFLuT = RHI::Image2D::Create(spec);

		auto pipeline = ShaderMap::GetComputePipeline("BRDFGeneration", false);
		RHI::DescriptorTableCreateInfo tableInfo{};
		tableInfo.shader = pipeline->GetShader();

		Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(tableInfo);
		descriptorTable->SetImageView("LUT", s_rendererData->defaultResources.BRDFLuT->GetView(), 0);
		
		Ref<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
		commandBuffer->Begin();

		commandBuffer->BindPipeline(pipeline);
		commandBuffer->BindDescriptorTable(descriptorTable);

		const uint32_t groupCount = Math::DivideRoundUp(BRDFSize, 32u);
		commandBuffer->Dispatch(groupCount, groupCount, 1);
		commandBuffer->End();
		commandBuffer->Execute();
	}
}
