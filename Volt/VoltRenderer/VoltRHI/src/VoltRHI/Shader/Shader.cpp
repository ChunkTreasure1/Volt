#include "rhipch.h"
#include "Shader.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Shader/VulkanShader.h>
#include <VoltD3D12/Shader/D3D12Shader.h>

namespace Volt::RHI
{
	ShaderUniform::ShaderUniform(const ShaderUniformType type, const size_t size, const size_t offset)
		: type(type), size(size), offset(offset)
	{
	}

	void ShaderDataBuffer::AddMember(const std::string& name, ShaderUniformType type, size_t size, size_t offset)
	{
		m_uniforms[name] = { type, size, offset };
	}

	void ShaderDataBuffer::SetSize(const size_t size)
	{
		assert(size <= 128);
		m_size = size;
	}

	Ref<Shader> Shader::Create(const ShaderSpecification& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRef<D3D12Shader>(createInfo); break;
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Vulkan: return CreateRef<VulkanShader>(createInfo); break;
		}

		return nullptr;
	}
}
