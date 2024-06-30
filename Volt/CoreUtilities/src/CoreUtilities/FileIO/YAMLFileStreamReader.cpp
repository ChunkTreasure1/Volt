#include "cupch.h"
#include "YAMLFileStreamReader.h"

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
	catch (std::exception&)
	{
		return false;
	}

	return true;
}
