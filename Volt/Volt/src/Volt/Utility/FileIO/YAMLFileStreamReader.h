#pragma once

#include "Volt/Utility/FileIO/YAMLStreamReader.h"

namespace Volt
{
	class YAMLFileStreamReader : public YAMLStreamReader
	{
	public:
		~YAMLFileStreamReader() override = default;
		const bool OpenFile(const std::filesystem::path& filePath);
	};
}
