#include "rhipch.h"
#include "Shader.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Shader/VulkanShader.h>

namespace Volt::RHI
{
	void ShaderDataBuffer::AddMember(std::string_view name, ShaderUniformType type, size_t size, size_t offset)
	{
		m_uniforms[name] = { type, size, offset };
	}

	Ref<Shader> Shader::Create(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanShader>(name, sourceFiles, forceCompile); break;
		}

		return nullptr;
	}

	Shader::Shader(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles)
		: m_name(name), m_sourceFiles(sourceFiles)
	{}
}
