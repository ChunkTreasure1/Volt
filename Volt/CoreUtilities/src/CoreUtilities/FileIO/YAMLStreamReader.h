#pragma once

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <functional>

class YAMLStreamReader
{
public:
	YAMLStreamReader();

	const bool OpenFile(const std::filesystem::path& filePath);

	const bool HasKey(const std::string& key);
	const bool IsSequenceEmpty(const std::string& key);

	void EnterScope(const std::string& key);
	void ExitScope();

	template<typename T>
	const T ReadAtKey(const std::string& key, const T& defaultValue);

	template<typename T>
	const T ReadValue();

	template<typename T>
	const T ReadKeyValue();

	void ForEach(const std::string& key, std::function<void()> function);

	const YAML::Node& GetRawNode() const { return m_currentNode; }

private:
	YAML::Node m_currentNode;
	YAML::Node m_currentSequenceKeyNode;

	std::vector<YAML::Node> m_nodeStack;

	YAML::Node m_rootNode;
};

template<typename T>
inline const T YAMLStreamReader::ReadAtKey(const std::string& key, const T& defaultValue)
{
	return m_currentNode[key] ? m_currentNode[key].as<T>() : defaultValue;
}

template<typename T>
inline const T YAMLStreamReader::ReadValue()
{
	return m_currentNode.as<T>();
}

template<typename T>
inline const T YAMLStreamReader::ReadKeyValue()
{
	return m_currentSequenceKeyNode.as<T>();
}
