#include "vtpch.h"
#include "YAMLFileStreamReader.h"

namespace Volt
{
	const bool YAMLFileStreamReader::OpenFile(const std::filesystem::path& filePath)
	{
		if (!std::filesystem::exists(filePath))
		{
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			return false;
		}

		std::stringstream strStream;
		strStream << file.rdbuf();
		file.close();

		try
		{
			m_rootNode = YAML::Load(strStream.str());
			m_currentNode = m_rootNode;
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("[YAMLFileStreamReader] File {0} contains invalid YAML! Please correct it! Error: {1}", filePath.string(), e.what());
			return false;
		}

		return true;
	}
}
