#include "cupch.h"
#include "YAMLStreamWriter.h"
#include "YAMLFileStreamWriter.h"

void YAMLStreamWriter::BeginMap()
{
	m_emitter << YAML::BeginMap;
}

void YAMLStreamWriter::EndMap()
{
	m_emitter << YAML::EndMap;
}

void YAMLStreamWriter::BeginSequence(const std::string& sequenceName)
{
	m_emitter << YAML::Key << sequenceName << YAML::BeginSeq;
}

void YAMLStreamWriter::EndSequence()
{
	m_emitter << YAML::EndSeq;
}

void YAMLStreamWriter::BeginMapNamned(const std::string& mapName)
{
	m_emitter << YAML::Key << mapName << YAML::BeginMap;
}
