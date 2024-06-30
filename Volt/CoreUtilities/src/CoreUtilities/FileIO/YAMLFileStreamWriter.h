#pragma once

#include "CoreUtilities/FileIO/YAMLStreamWriter.h"

class VTCOREUTIL_API YAMLFileStreamWriter : public YAMLStreamWriter
{
public:
	YAMLFileStreamWriter(const std::filesystem::path& targetFilePath);
	~YAMLFileStreamWriter() override = default;

	const bool WriteToDisk();

private:
	std::filesystem::path m_targetFilePath;
};
