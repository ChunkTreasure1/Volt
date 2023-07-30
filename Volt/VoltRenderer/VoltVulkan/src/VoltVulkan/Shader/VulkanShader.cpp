#include "vkpch.h"
#include "VulkanShader.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Shader/ShaderUtility.h>

namespace Volt::RHI
{
	VulkanShader::VulkanShader(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile)
		: Shader(name, sourceFiles)
	{
		if (sourceFiles.empty())
		{
			GraphicsContext::Log(Severity::Error, std::format("[VulkanShader] Trying to create a shader {0} without any sources!", name));
			return;
		}

		Reload(forceCompile);
	}

	VulkanShader::~VulkanShader()
	{
		Release();
	}

	const bool VulkanShader::Reload(bool forceCompile)
	{
		Utility::CreateCacheDirectoryIfNeeded();

		m_shaderSources.clear();
		LoadShaderFromFiles();

		std::unordered_map<ShaderStage, std::vector<uint32_t>> shaderData;
		if (!CompileOrGetBinary(shaderData, forceCompile))
		{
			return false;
		}

		Release();
		LoadAndCreateShaders(shaderData);
		ReflectAllStages(shaderData);

		return true;
	}

	void* VulkanShader::GetHandleImpl()
	{
		return nullptr;
	}

	void VulkanShader::LoadShaderFromFiles()
	{
		for (const auto& path : m_sourceFiles)
		{
			const ShaderStage stage = Utility::GetShaderStageFromFilename(path.filename().string());
			const std::string source = Utility::ReadStringFromFile(path);
	
			if (source.empty())
			{
				continue;
			}

			if (m_shaderSources.find(stage) != m_shaderSources.end())
			{
				GraphicsContext::Log(Severity::Error, std::format("[VulkanShader] Multiple shaders of same stage defined in file {0}!", path.string().c_str()));
				continue;
			}

			m_shaderSources[stage] = source;
		}
	}

	void VulkanShader::Release()
	{
		m_resources = {};
	}

	const bool VulkanShader::CompileOrGetBinary(std::unordered_map<ShaderStage, std::vector<uint32_t>>& outShaderData, bool forceCompile)
	{
		return false;
	}

	void VulkanShader::LoadAndCreateShaders(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData)
	{

	}

	void VulkanShader::ReflectAllStages(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData)
	{
	}

	void VulkanShader::ReflectStage(ShaderStage stage, const std::vector<uint32_t>& data)
	{
	}
}
