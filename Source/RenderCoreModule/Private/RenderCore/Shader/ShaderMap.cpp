#include "rcpch.h"
#include "RenderCore/Shader/ShaderMap.h"

#include <RHIModule/Shader/Shader.h>
#include <RHIModule/Pipelines/RenderPipeline.h>
#include <RHIModule/Pipelines/ComputePipeline.h>

#include <CoreUtilities/Math/Hash.h>
#include <CoreUtilities/Time/ScopedTimer.h>

namespace Volt
{
	namespace Utility
	{
		inline static const size_t GetComputeShaderHash(const std::string& name)
		{
			return std::hash<std::string>()(name);
		}

		inline static const size_t GetRenderPipelineHash(const RHI::RenderPipelineCreateInfo& pipelineInfo)
		{
			size_t hash = std::hash<std::string_view>()(pipelineInfo.shader->GetName());
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(pipelineInfo.topology)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(pipelineInfo.cullMode)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(pipelineInfo.fillMode)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(pipelineInfo.depthMode)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(pipelineInfo.depthCompareOperator)));
		
			return hash;
		}
	}

	ShaderMap::ShaderMap()
	{
		VT_ASSERT(s_instance == nullptr);
		s_instance = this;
	}

	ShaderMap::~ShaderMap()
	{
		m_shaderMap.clear();
		m_computePipelineCache.clear();
		m_renderPipelineCache.clear();

		s_instance = nullptr;
	}

	void ShaderMap::ReloadAll()
	{
		for (const auto [name, shader] : s_instance->m_shaderMap)
		{
			shader->Reload(true);
		}
	}

	bool ShaderMap::ReloadShaderByName(const std::string& name)
	{
		if (!s_instance->m_shaderMap.contains(name))
		{
			return false;
		}

		auto shader = s_instance->m_shaderMap.at(name);
		bool reloaded = shader->Reload(true);
	
		// #TODO_Ivar: Hack for finding out if it's a compute shader or not

		if (reloaded)
		{
			if (shader->GetSourceEntries().size() == 1)
			{
				for (const auto& [hash, pipeline] : s_instance->m_computePipelineCache)
				{
					if (pipeline->GetShader() == shader)
					{
						pipeline->Invalidate();
					}
				}
			}
			else
			{
				for (const auto& [hash, pipeline] : s_instance->m_renderPipelineCache)
				{
					if (pipeline->GetShader() == shader)
					{
						pipeline->Invalidate();
					}
				}
			}
		}

		return reloaded;
	}

	void ShaderMap::RegisterShader(const std::string& name, RefPtr<RHI::Shader> shader)
	{
		std::scoped_lock lock{ s_instance->m_registerMutex };
		s_instance->m_shaderMap[name] = shader;
	}

	RefPtr<RHI::Shader> ShaderMap::Get(const std::string& name)
	{
		VT_PROFILE_FUNCTION();

		if (!s_instance->m_shaderMap.contains(name))
		{
			return nullptr;
		}

		return s_instance->m_shaderMap.at(name);
	}

	RefPtr<RHI::ComputePipeline> ShaderMap::GetComputePipeline(const std::string& name, bool useGlobalResouces)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetComputeShaderHash(name);

		if (s_instance->m_computePipelineCache.contains(hash))
		{
			return s_instance->m_computePipelineCache.at(hash);
		}

		auto shader = Get(name);
		VT_ENSURE(shader);

		RefPtr<RHI::ComputePipeline> pipeline = RHI::ComputePipeline::Create(shader, useGlobalResouces);
		s_instance->m_computePipelineCache[hash] = pipeline;

		return pipeline;
	}

	RefPtr<RHI::RenderPipeline> ShaderMap::GetRenderPipeline(const RHI::RenderPipelineCreateInfo& pipelineInfo)
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(pipelineInfo.shader);

		const size_t hash = Utility::GetRenderPipelineHash(pipelineInfo);
		
		if (s_instance->m_renderPipelineCache.contains(hash))
		{
			return s_instance->m_renderPipelineCache.at(hash);
		}

		RefPtr<RHI::RenderPipeline> pipeline = RHI::RenderPipeline::Create(pipelineInfo);
		s_instance->m_renderPipelineCache[hash] = pipeline;

		return pipeline;
	}
}
