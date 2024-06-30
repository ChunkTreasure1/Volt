#include "cupch.h"
#include "YAMLFileStreamWriter.h"

	YAMLFileStreamWriter::YAMLFileStreamWriter(const std::filesystem::path& targetFilePath)
		: m_targetFilePath(targetFilePath)
	{
	}

	const bool YAMLFileStreamWriter::WriteToDisk()
	{
		std::ofstream fout{ m_targetFilePath };
		if (!fout.is_open())
		{
			return false;
		}

		fout << m_emitter.c_str();

		return true;
	}
