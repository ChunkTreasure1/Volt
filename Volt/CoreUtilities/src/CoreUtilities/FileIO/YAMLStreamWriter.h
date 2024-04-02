#pragma once

#include "CoreUtilities/FileIO/SerializationHelpers.h"

#include <yaml-cpp/yaml.h>
#include <filesystem>

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

	template<typename T>
	void AddValue(const T& value);

	inline YAML::Emitter& GetEmitter() { return m_emitter; }

protected:
	YAML::Emitter m_emitter;
};

template<typename K, typename T>
inline void YAMLStreamWriter::SetKey(const K& key, const T& value)
{
	m_emitter << YAML::Key << key << YAML::Value << value;
}

template<typename T>
inline void YAMLStreamWriter::AddValue(const T& value)
{
	m_emitter << value;
}
