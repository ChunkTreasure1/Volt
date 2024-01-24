#include "vtpch.h"
#include "RendererNew.h"

#include "Volt/Core/Application.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Utility/FunctionQueue.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

#include "Volt/Math/Math.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Graphics/Swapchain.h>

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

		CreaDefaultResources();
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

	void RendererNew::Update()
	{
		//GlobalResourceManager::Update();
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

	void RendererNew::CreaDefaultResources()
	{
		// Full white 1x1
		{
			constexpr uint32_t PIXEL_DATA = 0xffffffff;
			s_rendererData->defaultResources.whiteTexture = Texture2D::Create(RHI::PixelFormat::R8G8B8A8_UNORM, 1, 1, &PIXEL_DATA);
			s_rendererData->defaultResources.whiteTexture->handle = 0;
		}
	}
}
