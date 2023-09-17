#pragma once

#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	class YAMLStreamWriter
	{
	public:
		YAMLStreamWriter(const std::filesystem::path& targetFilePath);

		void BeginMap();
		void EndMap();

		void BeginSequence(const std::string& sequenceName);
		void EndSequence();

		void BeginMapNamned(const std::string& mapName);

		template<typename T>
		void SetKey(const std::string& key, const T& value);

		const bool WriteToDisk();

	private:
		std::filesystem::path m_targetFilePath;
		YAML::Emitter m_emitter;
	};
	
	template<typename T>
	inline void YAMLStreamWriter::SetKey(const std::string& key, const T& value)
	{
		m_emitter << YAML::Key << key << YAML::Value << value;
	}
}
