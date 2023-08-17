#include "vtpch.h"
#include "RendererNew.h"

#include "Volt/Core/Application.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Utility/FunctionQueue.h"

#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Graphics/Swapchain.h>

namespace Volt
{
	struct RendererData
	{
		Ref<RHI::ShaderCompiler> shaderCompiler;

		std::vector<FunctionQueue> deletionQueue;
	};

	Scope<RendererData> s_rendererData;

	void RendererNew::Initialize()
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

	void RendererNew::Shutdown()
	{
		for (auto& resourceQueue : s_rendererData->deletionQueue)
		{
			resourceQueue.Flush();
		}

		s_rendererData = nullptr;
	}

	void RendererNew::Flush()
	{
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();
		s_rendererData->deletionQueue.at(currentFrame).Flush();
	}

	const uint32_t RendererNew::GetFramesInFlight()
	{
		return Application::Get().GetWindow().GetSwapchain().GetFramesInFlight();
	}

	void RendererNew::DestroyResource(std::function<void()>&& function)
	{
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain().GetCurrentFrame();
		s_rendererData->deletionQueue.at(currentFrame).Push(std::move(function));
	}
}
