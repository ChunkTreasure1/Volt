#pragma once

#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	class YAMLStreamWriter
	{
	public:
		virtual ~YAMLStreamWriter() = default;

		void BeginMap();
		void EndMap();

		void BeginSequence(const std::string& sequenceName);
		void EndSequence();

		void BeginMapNamned(const std::string& mapName);

		template<typename K, typename T>
		void SetKey(const K& key, const T& value);

		inline YAML::Emitter& GetEmitter() { return m_emitter; }

	protected:
		YAML::Emitter m_emitter;
	};
	
	template<typename K, typename T>
	inline void YAMLStreamWriter::SetKey(const K& key, const T& value)
	{
		m_emitter << YAML::Key << key << YAML::Value << value;
	}
}
