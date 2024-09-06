#include "cupch.h"
#include "FileIO/YAMLStreamReader.h"

YAMLStreamReader::YAMLStreamReader()
{
	m_nodeStack.reserve(100);
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

		if (node.first)
		{
			m_currentSequenceKeyNode.reset(node.first);
		}

		function();
	}

	m_currentSequenceKeyNode.reset();
	m_currentNode.reset(tempNode);
}
