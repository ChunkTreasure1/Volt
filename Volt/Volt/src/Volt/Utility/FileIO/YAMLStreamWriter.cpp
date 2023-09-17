#include "vtpch.h"
#include "YAMLStreamWriter.h"

namespace Volt
{
	YAMLStreamWriter::YAMLStreamWriter(const std::filesystem::path& targetFilePath)
		: m_targetFilePath(targetFilePath)
	{

	}
	
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
		m_emitter << YAML::Key << mapName << YAML::Value;
		m_emitter << YAML::BeginMap;
	}

	const bool YAMLStreamWriter::WriteToDisk()
	{
		std::ofstream fout{ m_targetFilePath };
		if (!fout.is_open())
		{
			return false;
		}
		
		fout << m_emitter.c_str();

		return true;
	}
}
