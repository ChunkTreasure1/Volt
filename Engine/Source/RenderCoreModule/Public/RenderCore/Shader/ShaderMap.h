#pragma once

#include "RenderCore/Config.h"

#include <RHIModule/Pipelines/RenderPipeline.h>
#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Shader/Shader.h>

#include <CoreUtilities/Containers/Map.h>

#include <unordered_map>
#include <string>

namespace Volt
{
	namespace RHI
	{
		struct RenderPipelineCreateInfo;
	}

	class VTRC_API ShaderMap
	{
	public:
		ShaderMap();
		~ShaderMap();

		static void ReloadAll();
		static bool ReloadShaderByName(const std::string& name);

		static void RegisterShader(const std::string& name, RefPtr<RHI::Shader> shader);

		static RefPtr<RHI::Shader> Get(const std::string& name);
		static RefPtr<RHI::ComputePipeline> GetComputePipeline(const std::string& name, bool useGlobalResouces = true);
		static RefPtr<RHI::RenderPipeline> GetRenderPipeline(const RHI::RenderPipelineCreateInfo& pipelineInfo);

	private:
		inline static ShaderMap* s_instance = nullptr;

		vt::map<std::string, RefPtr<RHI::Shader>> m_shaderMap;
		vt::map<size_t, RefPtr<RHI::ComputePipeline>> m_computePipelineCache;
		vt::map<size_t, RefPtr<RHI::RenderPipeline>> m_renderPipelineCache;
		
		std::mutex m_registerMutex;
		std::mutex m_computeCacheMutex;
		std::mutex m_renderCacheMutex;
	};
}
