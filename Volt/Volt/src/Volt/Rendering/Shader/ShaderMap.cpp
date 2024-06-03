#include "vtpch.h"
#include "ShaderMap.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/ShaderDefinition.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/ScopedTimer.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Math/Math.h"

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

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

	void ShaderMap::Initialize()
	{
		LoadShaders();
	}

	void ShaderMap::Shutdown()
	{
		s_shaderMap.clear();
		s_computePipelineCache.clear();
		s_renderPipelineCache.clear();
	}

	void ShaderMap::ReloadAll()
	{
		for (const auto [name, shader] : s_shaderMap)
		{
			shader->Reload(true);
		}
	}

	RefPtr<RHI::Shader> ShaderMap::Get(const std::string& name)
	{
		VT_PROFILE_FUNCTION();

		if (!s_shaderMap.contains(name))
		{
			return nullptr;
		}

		return s_shaderMap.at(name);
	}

	RefPtr<RHI::ComputePipeline> ShaderMap::GetComputePipeline(const std::string& name, bool useGlobalResouces)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetComputeShaderHash(name);

		if (s_computePipelineCache.contains(hash))
		{
			return s_computePipelineCache.at(hash);
		}

		RefPtr<RHI::ComputePipeline> pipeline = RHI::ComputePipeline::Create(Get(name), useGlobalResouces);
		s_computePipelineCache[hash] = pipeline;

		return pipeline;
	}

	RefPtr<RHI::RenderPipeline> ShaderMap::GetRenderPipeline(const RHI::RenderPipelineCreateInfo& pipelineInfo)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetRenderPipelineHash(pipelineInfo);
		
		if (s_renderPipelineCache.contains(hash))
		{
			return s_renderPipelineCache.at(hash);
		}

		RefPtr<RHI::RenderPipeline> pipeline = RHI::RenderPipeline::Create(pipelineInfo);
		s_renderPipelineCache[hash] = pipeline;

		return pipeline;
	}

	void ShaderMap::LoadShaders()
	{
		const std::vector<std::filesystem::path> searchPaths =
		{
			ProjectManager::GetEngineDirectory() / "Engine" / "Shaders",
			ProjectManager::GetAssetsDirectory()
		};

		std::vector<std::future<void>> shaderFutures;

		std::mutex shaderMapMutex;

		ScopedTimer timer{};

		VT_CORE_INFO("[ShaderMap]: Starting shader import!");

		for (const auto& searchPath : searchPaths)
		{
			for (const auto& path : std::filesystem::recursive_directory_iterator(searchPath))
			{
				const auto relPath = AssetManager::GetRelativePath(path.path());
				const auto type = AssetManager::GetAssetTypeFromExtension(relPath.extension().string());
			
				if (type != AssetType::ShaderDefinition)
				{
					continue;
				}

				if (!AssetManager::ExistsInRegistry(relPath))
				{
					AssetManager::Get().AddAssetToRegistry(relPath);
				}

				Ref<ShaderDefinition> shaderDef = AssetManager::GetAsset<ShaderDefinition>(relPath);

				auto& threadPool = Application::GetThreadPool();

				shaderFutures.emplace_back(threadPool.SubmitTask([&, def = shaderDef]() 
				{
					// #TODO: Fix force compile!
					// We need to do this because the formats / input layouts are not created correctly otherwise

					RHI::ShaderSpecification specification;
					specification.entryPoint = def->GetEntryPoint();
					specification.name = def->GetName();
					specification.sourceFiles = def->GetSourceFiles();
					specification.permutations = def->GetPermutations();
					specification.forceCompile = false;

					RefPtr<RHI::Shader> shader = RHI::Shader::Create(specification);
					{
						std::scoped_lock lock{ shaderMapMutex };
						s_shaderMap[std::string(def->GetName())] = shader;
					}
				}));
			}
		}

		for (const auto& future : shaderFutures)
		{
			future.wait();
		}

		VT_CORE_INFO("[ShaderMap]: Shader import finished in {0} seconds!", timer.GetTime<Time::Seconds>());
	}

	void ShaderMap::RegisterShader(const std::string& name, RefPtr<RHI::Shader> shader)
	{
		std::scoped_lock lock{ s_registerMutex };
		s_shaderMap[std::string(name)] = shader;
	}
}
