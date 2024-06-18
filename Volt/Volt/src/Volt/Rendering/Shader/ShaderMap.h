#pragma once

#include "Volt/Core/Base.h"

#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Shader/Shader.h>

#include <CoreUtilities/Containers/Map.h>

#include <unordered_map>
#include <string>

namespace Volt
{
	namespace RHI
	{
		struct RenderPipelineCreateInfo;
	}

	class ShaderMap
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void ReloadAll();
		static bool ReloadShaderByName(const std::string& name);

		static RefPtr<RHI::Shader> Get(const std::string& name);
		static RefPtr<RHI::ComputePipeline> GetComputePipeline(const std::string& name, bool useGlobalResouces = true);
		static RefPtr<RHI::RenderPipeline> GetRenderPipeline(const RHI::RenderPipelineCreateInfo& pipelineInfo);

	private:
		static void LoadShaders();
		static std::vector<std::filesystem::path> FindShaderIncludes(const std::filesystem::path& filePath);
		static void RegisterShader(const std::string& name, RefPtr<RHI::Shader> shader);

		inline static vt::map<std::string, RefPtr<RHI::Shader>> s_shaderMap;

		inline static vt::map<size_t, RefPtr<RHI::ComputePipeline>> s_computePipelineCache;
		inline static vt::map<size_t, RefPtr<RHI::RenderPipeline>> s_renderPipelineCache;

		inline static std::mutex s_registerMutex;
	};
}
