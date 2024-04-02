#include "cupch.h"
#include "YAMLMemoryStreamReader.h"

#include "CoreUtilities/Buffer/Buffer.h"

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
	catch (std::exception&)
	{
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
