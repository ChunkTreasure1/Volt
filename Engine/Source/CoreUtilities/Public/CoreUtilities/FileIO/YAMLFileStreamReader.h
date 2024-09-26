#pragma once

#include "CoreUtilities/FileIO/YAMLStreamReader.h"

class VTCOREUTIL_API YAMLFileStreamReader : public YAMLStreamReader
{
public:
	~YAMLFileStreamReader() override = default;
	const bool OpenFile(const std::filesystem::path& filePath);
};
