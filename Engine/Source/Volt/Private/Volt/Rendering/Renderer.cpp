#include "vtpch.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Core/Application.h"
#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/Rendering/ShaderSource.h"
#include "Volt/Asset/Rendering/ShaderDefinition.h"

#include "Volt/Project/ProjectManager.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/Math/Math.h"

#include <AssetSystem/AssetManager.h>

#include <JobSystem/TaskGraph.h>

#include <RenderCore/RenderGraph/RenderGraphExecutionThread.h>
#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/Debug/ShaderRuntimeValidator.h>
#include <RenderCore/Resources/BindlessResourcesManager.h>
#include <RenderCore/Shader/ShaderMap.h>

#include <RHIModule/Shader/ShaderCompiler.h>
#include <RHIModule/Shader/ShaderCache.h>
#include <RHIModule/Images/SamplerState.h>
#include <RHIModule/Graphics/Swapchain.h>
#include <RHIModule/Images/Image.h>
#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Images/ImageUtility.h>
#include <RHIModule/Descriptors/DescriptorTable.h>
#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Utility/ResourceUtility.h>

#include <CoreUtilities/Containers/FunctionQueue.h>
#include <CoreUtilities/Math/Hash.h>
#include <CoreUtilities/Time/ScopedTimer.h>

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

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
		~RendererData()
		{
			defaultResources.Clear();
			samplers.clear();

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
			shaderValidator = nullptr;
#endif
			shaderCache = nullptr;
			shaderCompiler = nullptr;
			shaderMap = nullptr;
			bindlessResourcesManager = nullptr;

			for (auto& resourceQueue : deletionQueue)
			{
				resourceQueue.Flush();
			}
		}

		RefPtr<RHI::ShaderCompiler> shaderCompiler;
		Scope<RHI::ShaderCache> shaderCache;
		Scope<ShaderMap> shaderMap;
		Scope<BindlessResourcesManager> bindlessResourcesManager;

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		Scope<ShaderRuntimeValidator> shaderValidator;
#endif

		Vector<FunctionQueue> deletionQueue;
		std::unordered_map<size_t, BindlessResourceRef<RHI::SamplerState>> samplers;

		DefaultResources defaultResources;
	};

	Scope<RendererData> s_rendererData;

	void Renderer::PreInitialize()
	{
		s_rendererData = CreateScope<RendererData>();
		s_rendererData->deletionQueue.resize(WindowManager::Get().GetMainWindow().GetSwapchain().GetFramesInFlight());

		// Create shader compiler
		{
			RHI::ShaderCompilerCreateInfo shaderCompilerInfo{};
			shaderCompilerInfo.flags = RHI::ShaderCompilerFlags::WarningsAsErrors;

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
			shaderCompilerInfo.flags |= RHI::ShaderCompilerFlags::EnableShaderValidator;
#endif

			shaderCompilerInfo.includeDirectories =
			{
				ProjectManager::GetEngineShaderIncludeDirectory(),
				ProjectManager::GetAssetsDirectory()
			};

			s_rendererData->shaderCompiler = RHI::ShaderCompiler::Create(shaderCompilerInfo);
		}

		// Create shader cache
		{
			RHI::ShaderCacheCreateInfo info{};
			info.cacheDirectory = "Engine/Shaders/Cache";

			s_rendererData->shaderCache = CreateScope<RHI::ShaderCache>(info);
		}

		// Bindless resources manager
		{
			s_rendererData->bindlessResourcesManager = CreateScope<BindlessResourcesManager>();
		}
	}

	void Renderer::Initialize()
	{
		s_rendererData->shaderMap = CreateScope<ShaderMap>();
		LoadShaders();

		RenderGraphExecutionThread::Initialize(RenderGraphExecutionThread::ExecutionMode::Multithreaded);

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		s_rendererData->shaderValidator = CreateScope<ShaderRuntimeValidator>();
#endif

		CreateDefaultResources();
	}

	void Renderer::Shutdown()
	{
		RenderGraphExecutionThread::Shutdown();
		s_rendererData = nullptr;
	}

	void Renderer::Flush()
	{
		const uint32_t currentFrame = WindowManager::Get().GetMainWindow().GetSwapchain().GetCurrentFrame();

		//Application::GetThreadPool().SubmitTask([currentFrame, queueCopy = s_rendererData->deletionQueue.at(currentFrame)]() mutable
		//{
		//	queueCopy.Flush();
		//});

		s_rendererData->deletionQueue.at(currentFrame).Flush();
		s_rendererData->bindlessResourcesManager->Update();
	}

	const uint32_t Renderer::GetFramesInFlight()
	{
		return WindowManager::Get().GetMainWindow().GetSwapchain().GetFramesInFlight();
	}

	void Renderer::DestroyResource(std::function<void()>&& function)
	{
		if (!s_rendererData)
		{
			function();
			return;
		}

		const uint32_t currentFrame = WindowManager::Get().GetMainWindow().GetSwapchain().GetCurrentFrame();
		s_rendererData->deletionQueue.at(currentFrame).Push(std::move(function));
	}

	const DefaultResources& Renderer::GetDefaultResources()
	{
		return s_rendererData->defaultResources;
	}

	SceneEnvironment Renderer::GenerateEnvironmentTextures(AssetHandle baseTextureHandle)
	{
		Ref<Texture2D> environmentTexture = AssetManager::GetAsset<Texture2D>(baseTextureHandle);
		if (!environmentTexture || !environmentTexture->IsValid())
		{
			return {};
		}

		constexpr uint32_t CUBE_MAP_SIZE = 1024;
		constexpr uint32_t IRRADIANCE_MAP_SIZE = 32;
		constexpr uint32_t CONVERSION_THREAD_GROUP_SIZE = 32;

		RefPtr<RHI::Image> environmentUnfiltered;
		RefPtr<RHI::Image> environmentFiltered;
		RefPtr<RHI::Image> irradianceMap;

		auto linearSampler = GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>();

		RefPtr<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
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

			environmentUnfiltered = RHI::Image::Create(imageSpec);

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

			RefPtr<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(tableInfo);
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
			imageSpec.debugName = "Environment - Radiance";

			environmentFiltered = RHI::Image::Create(imageSpec);

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

			Vector<RefPtr<RHI::DescriptorTable>> descriptorTables;
			for (uint32_t i = 0; i < imageSpec.mips; i++)
			{
				descriptorTables.emplace_back(RHI::DescriptorTable::Create(tableInfo));
				descriptorTables.back()->SetImageView("u_input", environmentUnfiltered->GetView(), 0);
				descriptorTables.back()->SetSamplerState("u_linearSampler", linearSampler->GetResource(), 0);
			}

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

				descriptorTables[i]->SetImageView("o_output", environmentFiltered->GetArrayView(i), 0);

				commandBuffer->BindPipeline(pipeline);
				commandBuffer->BindDescriptorTable(descriptorTables[i]);
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
			imageSpec.debugName = "Environment - Irradiance";

			irradianceMap = RHI::Image::Create(imageSpec);
		
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

			RefPtr<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(tableInfo);
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

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
	ShaderRuntimeValidator& Renderer::GetRuntimeShaderValidator()
	{
		return *s_rendererData->shaderValidator;
	}
#endif

	void Renderer::Update()
	{
	}

	void Renderer::EndOfFrameUpdate()
	{
#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		//s_rendererData->shaderValidator->ReadbackErrorBuffer();

		const auto& frameErrors = s_rendererData->shaderValidator->GetValidationErrors();
		for (const auto& error : frameErrors)
		{
			VT_LOGC(Error, LogRender, error);
		}
#endif
	}

	BindlessResourceRef<RHI::SamplerState> Renderer::GetSamplerInternal(const RHI::SamplerStateCreateInfo& samplerInfo)
	{
		const size_t hash = Utility::GetHashFromSamplerInfo(samplerInfo);
		if (s_rendererData->samplers.contains(hash))
		{
			return s_rendererData->samplers.at(hash);
		}

		BindlessResourceRef<RHI::SamplerState> samplerState = BindlessResource<RHI::SamplerState>::CreateRef(samplerInfo);
		s_rendererData->samplers[hash] = samplerState;

		return samplerState;
	}

	void Renderer::CreateDefaultResources()
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
			imageSpec.debugName = "BlackCube";

			s_rendererData->defaultResources.blackCubeTexture = RHI::Image::Create(imageSpec, PIXEL_DATA);
		}

		GenerateBRDFLuT();

		// Default material
		{
			s_rendererData->defaultResources.defaultMaterial = AssetManager::CreateMemoryAsset<Material>("DefaultMaterial", ShaderMap::GetComputePipeline("OpaqueDefault"));
		}
	}
	
	void Renderer::GenerateBRDFLuT()
	{
		constexpr uint32_t BRDFSize = 512;

		RHI::ImageSpecification spec{};
		spec.format = RHI::PixelFormat::R16G16_SFLOAT;
		spec.usage = RHI::ImageUsage::AttachmentStorage;
		spec.width = BRDFSize;
		spec.height = BRDFSize;
		spec.debugName = "BRDFLut";

		s_rendererData->defaultResources.BRDFLuT = RHI::Image::Create(spec);

		RefPtr<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();

		RenderGraph renderGraph{ commandBuffer };
		RenderGraphImageHandle targetImageHandle = renderGraph.AddExternalImage(s_rendererData->defaultResources.BRDFLuT);

		renderGraph.AddPass("BRDF Pass", 
		[&](RenderGraph::Builder& builder) 
		{
			builder.WriteResource(targetImageHandle);
			builder.SetHasSideEffect();
		},
		[=](RenderContext& context) 
		{
			RenderingInfo renderingInfo = context.CreateRenderingInfo(BRDFSize, BRDFSize, { targetImageHandle });

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("GenerateBRDF");
			pipelineInfo.cullMode = RHI::CullMode::None;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);
			
			context.BeginRendering(renderingInfo);
			RCUtils::DrawFullscreenTriangle(context, pipeline);
			context.EndRendering();
		});

		renderGraph.Compile();
		renderGraph.ExecuteImmediateAndWait();
	}

	static Vector<std::filesystem::path> FindShaderIncludes(const std::filesystem::path& filePath)
	{
		constexpr const char* INCLUDE_KEYWORD = "#include";

		std::ifstream input{ filePath };
		if (!input.is_open())
		{
			return {};
		}

		std::string shaderString{};
		input.seekg(0, std::ios::end);
		shaderString.resize(input.tellg());
		input.seekg(0, std::ios::beg);
		input.read(&shaderString[0], shaderString.size());

		input.close();

		Vector<std::filesystem::path> resultIncludes{};

		size_t offset = shaderString.find(INCLUDE_KEYWORD, 0);
		while (offset != std::string::npos)
		{
			size_t openOffset = shaderString.find_first_of("\"<", offset);
			if (openOffset == std::string::npos)
			{
				break;
			}

			size_t closeOffset = shaderString.find_first_of("\">", openOffset + 1);
			if (closeOffset == std::string::npos)
			{
				break;
			}

			std::string includeString = shaderString.substr(openOffset + 1, closeOffset - openOffset - 1);

			// Find real path
			if (std::filesystem::exists(filePath.parent_path() / includeString))
			{
				resultIncludes.emplace_back(filePath.parent_path() / includeString);
			}
			else if (std::filesystem::exists(ProjectManager::GetEngineShaderIncludeDirectory() / includeString))
			{
				resultIncludes.emplace_back(ProjectManager::GetEngineShaderIncludeDirectory() / includeString);
			}
			else if (std::filesystem::exists(ProjectManager::GetAssetsDirectory() / includeString))
			{
				resultIncludes.emplace_back(ProjectManager::GetAssetsDirectory() / includeString);
			}

			offset = shaderString.find(INCLUDE_KEYWORD, offset + 1);
		}

		return resultIncludes;
	}

	void Renderer::LoadShaders()
	{
		const Vector<std::filesystem::path> searchPaths =
		{
			ProjectManager::GetEngineDirectory() / "Engine" / "Shaders",
			ProjectManager::GetAssetsDirectory()
		};

		ScopedTimer timer{};

		VT_LOGC(Info, LogRender, "Starting shader import!");

		// Add source files to asset registry and setup dependencies
		for (const auto& searchPath : searchPaths)
		{
			for (const auto& path : std::filesystem::recursive_directory_iterator(searchPath))
			{
				const auto relPath = AssetManager::GetRelativePath(path.path());
				const auto extStr = relPath.extension().string();

				if (extStr != ShaderSource::Extension && extStr != ShaderSource::ExtensionInclude)
				{
					continue;
				}

				AssetHandle shaderHandle = AssetManager::Get().GetOrAddAssetToRegistry(relPath, AssetTypes::ShaderSource);
				if (shaderHandle == Asset::Null())
				{
					continue;
				}

				const auto includes = FindShaderIncludes(relPath);
				for (const auto include : includes)
				{
					const auto relIncludePath = AssetManager::GetRelativePath(include);

					AssetHandle includeHandle = AssetManager::Get().GetOrAddAssetToRegistry(relIncludePath, AssetTypes::ShaderSource);
					if (includeHandle != Asset::Null())
					{
						AssetManager::AddDependencyToAsset(shaderHandle, includeHandle);
					}
				}
			}
		}

		TaskGraph taskGraph{};

		for (const auto& searchPath : searchPaths)
		{
			for (const auto& path : std::filesystem::recursive_directory_iterator(searchPath))
			{
				const auto relPath = AssetManager::GetRelativePath(path.path());
				if (relPath.extension().string() != ShaderDefinition::Extension)
				{
					continue;
				}

				AssetHandle defHandle = AssetManager::Get().GetOrAddAssetToRegistry(relPath, AssetTypes::ShaderDefinition);
				Ref<ShaderDefinition> shaderDef = AssetManager::GetAsset<ShaderDefinition>(defHandle);

				for (const auto& sourceEntry : shaderDef->GetSourceEntries())
				{
					AssetManager::AddDependencyToAsset(defHandle, AssetManager::GetAssetHandleFromFilePath(sourceEntry.filePath));
				}

				taskGraph.AddTask([&, def = shaderDef]()
				{
					RHI::ShaderSpecification specification;
					specification.name = def->GetName();
					specification.sourceEntries = def->GetSourceEntries();
					specification.permutations = def->GetPermutations();
					specification.forceCompile = false;

					RefPtr<RHI::Shader> shader = RHI::Shader::Create(specification);
					ShaderMap::RegisterShader(std::string(def->GetName()), shader);
				});
			}
		}

		taskGraph.ExecuteAndWait();

		VT_LOGC(Info, LogRender, "Shader import finished in {} seconds!", timer.GetTime<Time::Seconds>());
	}
}
