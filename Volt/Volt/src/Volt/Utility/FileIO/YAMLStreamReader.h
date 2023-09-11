#pragma once

#include <filesystem>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	class YAMLStreamReader
	{
	public:
		YAMLStreamReader() = default;

		const bool OpenFile(const std::filesystem::path& filePath);

		const bool HasKey(const std::string& key);

		void EnterScope(const std::string& key);
		void ExitScope();

		template<typename T>
		const T ReadKey(const std::string& key, const T& defaultValue);

	private:
		YAML::Node m_currentNode;
		std::vector<YAML::Node> m_nodeStack;

		YAML::Node m_rootNode;
	};

	template<typename T>
	inline const T YAMLStreamReader::ReadKey(const std::string& key, const T& defaultValue)
	{
		return m_currentNode[key] ?  m_currentNode[key].as<T>() : defaultValue;
	}
}
