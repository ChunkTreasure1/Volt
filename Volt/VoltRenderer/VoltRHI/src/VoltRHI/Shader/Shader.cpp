#include "rhipch.h"
#include "Shader.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	ShaderUniform::ShaderUniform(const ShaderUniformType type, const size_t size, const size_t offset)
		: type(type), size(size), offset(offset)
	{
	}

	ShaderDataBuffer::ShaderDataBuffer(const ShaderDataBuffer& rhs)
	{
		m_uniforms = rhs.m_uniforms;
		m_size = rhs.m_size;
		memcpy_s(m_data, m_size, rhs.m_data, rhs.m_size);
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

	ShaderDataBuffer& ShaderDataBuffer::operator=(const ShaderDataBuffer& rhs)
	{
		m_uniforms = rhs.m_uniforms;
		m_size = rhs.m_size;
		memcpy_s(m_data, m_size, rhs.m_data, rhs.m_size);

		return *this;
	}

	RefPtr<Shader> Shader::Create(const ShaderSpecification& createInfo)
	{
		return RHIProxy::GetInstance().CreateShader(createInfo);
	}
}
