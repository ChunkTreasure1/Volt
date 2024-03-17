#pragma once

#include "Volt/Core/Base.h"

#include <unordered_map>
#include <string>

namespace Volt
{
	namespace RHI
	{
		class Shader;
		class ComputePipeline;
		class RenderPipeline;
		struct RenderPipelineCreateInfo;
	}

	class ShaderMap
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void ReloadAll();

		static Ref<RHI::Shader> Get(const std::string& name);
		static Ref<RHI::ComputePipeline> GetComputePipeline(const std::string& name, bool useGlobalResouces = true);
		static Ref<RHI::RenderPipeline> GetRenderPipeline(const RHI::RenderPipelineCreateInfo& pipelineInfo);

	private:
		static void LoadShaders();
		static void RegisterShader(const std::string& name, Ref<RHI::Shader> shader);

		inline static std::unordered_map<std::string, Ref<RHI::Shader>> s_shaderMap;

		inline static std::unordered_map<size_t, Ref<RHI::ComputePipeline>> s_computePipelineCache;
		inline static std::unordered_map<size_t, Ref<RHI::RenderPipeline>> s_renderPipelineCache;

		inline static std::mutex s_registerMutex;
	};
}
