#include "vtpch.h"
#include "YAMLMemoryStreamReader.h"

#include "Volt/Core/Buffer.h"
#include "Volt/Log/Log.h"

namespace Volt
{
	const bool YAMLMemoryStreamReader::ReadBuffer(const Buffer& buffer)
	{
		if (!buffer.IsValid())
		{
			return false;
		}

		std::string tempStr;
		tempStr.resize(buffer.GetSize());
		
		memcpy_s(tempStr.data(), tempStr.size(), buffer.As<void>(), buffer.GetSize());
		
		try
		{
			m_rootNode = YAML::Load(tempStr);
			m_currentNode = m_rootNode;
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("[YAMLStreamReader] Buffer contains invalid YAML! Please correct it! Error: {0}", e.what());
			return false;
		}

		return true;
	}

	const bool YAMLMemoryStreamReader::ConsumeBuffer(Buffer& buffer)
	{
		const bool success = ReadBuffer(buffer);
		buffer.Release();

		return success;
	}
}
