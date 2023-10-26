#include "vtpch.h"
#include "RendererNew.h"

#include "Volt/Core/Application.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Utility/FunctionQueue.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Graphics/Swapchain.h>

namespace Volt
{
	struct Samplers
	{
		Ref<RHI::SamplerState> linear;
		Ref<RHI::SamplerState> linearPoint;

		Ref<RHI::SamplerState> point;
		Ref<RHI::SamplerState> pointLinear;

		Ref<RHI::SamplerState> linearClamp;
		Ref<RHI::SamplerState> linearPointClamp;

		Ref<RHI::SamplerState> pointClamp;
		Ref<RHI::SamplerState> pointLinearClamp;

		Ref<RHI::SamplerState> anisotropic;
	};

	struct RendererData
	{
		Ref<RHI::ShaderCompiler> shaderCompiler;
		std::vector<FunctionQueue> deletionQueue;

		// Data
		Samplers samplers{};
		SamplersData samplersData{};

		inline void Shutdown()
		{
			samplers = {};
			samplersData = {};
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

		RegisterSamplers();

		RenderGraphExecutionThread::Initialize();
	}

	void RendererNew::Shutdown()
	{
		RenderGraphExecutionThread::Shutdown();

		UnregisterSamplers();

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

	void RendererNew::Update()
	{
		//GlobalResourceManager::Update();
	}

	SamplersData RendererNew::GetSamplersData()
	{
		return SamplersData();
	}

	template<RHI::TextureFilter min, RHI::TextureFilter mag, RHI::TextureFilter mip, RHI::TextureWrap wrapMode = RHI::TextureWrap::Repeat, RHI::AnisotropyLevel aniso = RHI::AnisotropyLevel::None>
	inline static Ref<RHI::SamplerState> CreateSamplerState()
	{
		RHI::SamplerStateCreateInfo info{};
		info.minFilter = min;
		info.magFilter = mag;
		info.mipFilter = mip;
		info.wrapMode = wrapMode;
		info.anisotropyLevel = aniso;

		return RHI::SamplerState::Create(info);
	}

	void RendererNew::RegisterSamplers()
	{
		// Create samplers
		{
			s_rendererData->samplers.linear = CreateSamplerState<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>();
			s_rendererData->samplers.linearPoint = CreateSamplerState<RHI::TextureFilter::Linear, RHI::TextureFilter::Nearest, RHI::TextureFilter::Linear>();

			s_rendererData->samplers.point = CreateSamplerState<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest>();
			s_rendererData->samplers.pointLinear = CreateSamplerState<RHI::TextureFilter::Nearest, RHI::TextureFilter::Linear, RHI::TextureFilter::Nearest>();

			s_rendererData->samplers.linearClamp = CreateSamplerState<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Clamp>();
			s_rendererData->samplers.linearPointClamp = CreateSamplerState<RHI::TextureFilter::Linear, RHI::TextureFilter::Nearest, RHI::TextureFilter::Linear, RHI::TextureWrap::Clamp>();

			s_rendererData->samplers.pointClamp = CreateSamplerState<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();
			s_rendererData->samplers.pointLinearClamp = CreateSamplerState<RHI::TextureFilter::Nearest, RHI::TextureFilter::Linear, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();
		
			s_rendererData->samplers.anisotropic = CreateSamplerState<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Repeat, RHI::AnisotropyLevel::X16>();
		}

		s_rendererData->samplersData.linearSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.linear);
		s_rendererData->samplersData.linearPointSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.linearPoint);
		s_rendererData->samplersData.pointSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.point);
		s_rendererData->samplersData.pointLinearSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.pointLinear);
		s_rendererData->samplersData.linearClampSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.linearClamp);
		s_rendererData->samplersData.linearPointClampSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.linearPointClamp);
		s_rendererData->samplersData.pointClampSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.pointClamp);
		s_rendererData->samplersData.pointLinearClampSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.pointLinearClamp);
		s_rendererData->samplersData.anisotropicSampler = GlobalResourceManager::RegisterResource<RHI::SamplerState>(s_rendererData->samplers.anisotropic);
	}

	void RendererNew::UnregisterSamplers()
	{
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.linearSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.linearPointSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.pointSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.pointLinearSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.linearClampSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.linearPointClampSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.pointClampSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.pointLinearClampSampler);
		GlobalResourceManager::UnregisterResource<RHI::SamplerState>(s_rendererData->samplersData.anisotropicSampler);
	}
}
