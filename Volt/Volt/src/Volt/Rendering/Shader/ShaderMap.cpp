#include "vtpch.h"
#include "ShaderMap.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/ShaderDefinition.h"

#include "Volt/Core/Application.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Math/Math.h"

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

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

	bool ShaderMap::ReloadShaderByName(const std::string& name)
	{
		if (!s_shaderMap.contains(name))
		{
			return false;
		}

		auto shader = s_shaderMap.at(name);
		bool reloaded = shader->Reload(true);
	
		// #TODO_Ivar: Hack for finding out if it's a compute shader or not

		if (reloaded)
		{
			if (shader->GetSourceEntries().size() == 1)
			{
				for (const auto& [hash, pipeline] : s_computePipelineCache)
				{
					if (pipeline->GetShader() == shader)
					{
						pipeline->Invalidate();
					}
				}
			}
			else
			{
				for (const auto& [hash, pipeline] : s_renderPipelineCache)
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

		auto shader = Get(name);
		VT_ENSURE(shader);

		RefPtr<RHI::ComputePipeline> pipeline = RHI::ComputePipeline::Create(shader, useGlobalResouces);
		s_computePipelineCache[hash] = pipeline;

		return pipeline;
	}

	RefPtr<RHI::RenderPipeline> ShaderMap::GetRenderPipeline(const RHI::RenderPipelineCreateInfo& pipelineInfo)
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(pipelineInfo.shader);

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
		const Vector<std::filesystem::path> searchPaths =
		{
			ProjectManager::GetEngineDirectory() / "Engine" / "Shaders",
			ProjectManager::GetAssetsDirectory()
		};

		ScopedTimer timer{};

		VT_CORE_INFO("[ShaderMap]: Starting shader import!");

		for (const auto& searchPath : searchPaths)
		{
			for (const auto& path : std::filesystem::recursive_directory_iterator(searchPath))
			{
				const auto relPath = AssetManager::GetRelativePath(path.path());
				const auto type = AssetManager::GetAssetTypeFromExtension(relPath.extension().string());

				if (type != AssetType::ShaderSource)
				{
					continue;
				}

				if (!AssetManager::ExistsInRegistry(relPath))
				{
					AssetManager::Get().AddAssetToRegistry(relPath);
				}

				AssetHandle shaderHandle = AssetManager::GetAssetHandleFromFilePath(relPath);
				if (shaderHandle == Asset::Null())
				{
					continue;
				}

				const auto includes = FindShaderIncludes(relPath);
				for (const auto include : includes)
				{
					const auto relIncludePath = AssetManager::GetRelativePath(include);

					if (!AssetManager::ExistsInRegistry(relIncludePath))
					{
						AssetManager::Get().AddAssetToRegistry(relIncludePath);
					}

					AssetHandle includeHandle = AssetManager::GetAssetHandleFromFilePath(relIncludePath);
					if (includeHandle != Asset::Null())
					{
						AssetManager::AddDependencyToAsset(shaderHandle, includeHandle);
					}
				}
			}
		}

		Vector<std::future<void>> shaderFutures;
		std::mutex shaderMapMutex;

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
				for (const auto& sourceEntry : shaderDef->GetSourceEntries())
				{
					AssetManager::AddDependencyToAsset(shaderDef->handle, AssetManager::GetAssetHandleFromFilePath(sourceEntry.filePath));
				}

				auto& threadPool = Application::GetThreadPool();

				shaderFutures.emplace_back(threadPool.SubmitTask([&, def = shaderDef]() 
				{
					RHI::ShaderSpecification specification;
					specification.name = def->GetName();
					specification.sourceEntries = def->GetSourceEntries();
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

	Vector<std::filesystem::path> ShaderMap::FindShaderIncludes(const std::filesystem::path& filePath)
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

	void ShaderMap::RegisterShader(const std::string& name, RefPtr<RHI::Shader> shader)
	{
		std::scoped_lock lock{ s_registerMutex };
		s_shaderMap[std::string(name)] = shader;
	}
}
