#pragma once

#include "Volt/Utility/FileIO/YAMLStreamReader.h"

namespace Volt
{
	class Buffer;

	class YAMLMemoryStreamReader : public YAMLStreamReader
	{
	public:
		~YAMLMemoryStreamReader() override = default;

		const bool ReadBuffer(const Buffer& buffer);
	};
}
