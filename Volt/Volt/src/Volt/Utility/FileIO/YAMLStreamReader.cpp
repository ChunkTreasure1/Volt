#include "vtpch.h"
#include "YAMLStreamReader.h"

#include "Volt/Log/Log.h"

#include "Volt/Core/Profiling.h"

namespace Volt
{
	YAMLStreamReader::YAMLStreamReader()
	{
		m_nodeStack.reserve(100);
	}

	const bool YAMLStreamReader::OpenFile(const std::filesystem::path& filePath)
	{
		VT_PROFILE_FUNCTION();

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
			VT_PROFILE_SCOPE("Parse YAML");
			m_rootNode = YAML::Load(strStream.str());
			m_currentNode = m_rootNode;
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("[YAMLStreamReader] File {0} contains invalid YAML! Please correct it! Error: {1}", filePath.string(), e.what());
			return false;
		}

		return true;
	}

	const bool YAMLStreamReader::HasKey(const std::string& key)
	{
		if (m_currentNode[key])
		{
			return true;
		}

		return false;
	}

	const bool YAMLStreamReader::IsSequenceEmpty(const std::string& key)
	{
		return m_currentNode[key].IsSequence() && m_currentNode[key].size() == 0;
	}

	void YAMLStreamReader::EnterScope(const std::string& key)
	{
		m_nodeStack.emplace_back(m_currentNode);
		m_currentNode.reset(m_nodeStack.back()[key]);
	}

	void YAMLStreamReader::ExitScope()
	{
		if (m_nodeStack.empty())
		{
			return;
		}

		m_currentNode.reset(m_nodeStack.back());
		m_nodeStack.pop_back();
	}

	void YAMLStreamReader::ForEach(const std::string& key, std::function<void()> function)
	{
		if (!m_currentNode[key])
		{
			return;
		}

		YAML::Node tempNode;
		tempNode.reset(m_currentNode);

		for (const auto& node : m_currentNode[key])
		{
			m_currentNode.reset(static_cast<YAML::Node>(node));

			function();
		}

		m_currentNode.reset(tempNode);
	}
}
