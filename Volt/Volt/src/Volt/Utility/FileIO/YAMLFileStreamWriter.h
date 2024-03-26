#pragma once

#include "Volt/Utility/FileIO/YAMLStreamWriter.h"

namespace Volt
{
	class YAMLFileStreamWriter : public YAMLStreamWriter
	{
	public:
		YAMLFileStreamWriter(const std::filesystem::path& targetFilePath);
		~YAMLFileStreamWriter() override = default;
	
		const bool WriteToDisk();
	
	private:
		std::filesystem::path m_targetFilePath;
	};
}
