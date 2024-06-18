#include "rhipch.h"
#include "ShaderCommon.h"

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

	void ShaderDataBuffer::Serialize(BinaryStreamWriter& streamWriter, const ShaderDataBuffer& data)
	{
		streamWriter.Write(data.m_uniforms);
		streamWriter.Write(data.m_data);
		streamWriter.Write(data.m_size);
	}

	void ShaderDataBuffer::Deserialize(BinaryStreamReader& streamReader, ShaderDataBuffer& outData)
	{
		streamReader.Read(outData.m_uniforms);
		streamReader.Read(outData.m_data);
		streamReader.Read(outData.m_size);
	}

	void ShaderConstantData::Serialize(BinaryStreamWriter& streamWriter, const ShaderConstantData& data)
	{
		streamWriter.Write(data.size);
		streamWriter.Write(data.offset);
		streamWriter.Write(data.stageFlags);
	}

	void ShaderConstantData::Deserialize(BinaryStreamReader& streamReader, ShaderConstantData& outData)
	{
		streamReader.Read(outData.size);
		streamReader.Read(outData.offset);
		streamReader.Read(outData.stageFlags);
	}

	void ShaderResourceBinding::Serialize(BinaryStreamWriter& streamWriter, const ShaderResourceBinding& data)
	{
		streamWriter.Write(data.set);
		streamWriter.Write(data.binding);
	}

	void ShaderResourceBinding::Deserialize(BinaryStreamReader& streamReader, ShaderResourceBinding& outData)
	{
		streamReader.Read(outData.set);
		streamReader.Read(outData.binding);
	}
}

