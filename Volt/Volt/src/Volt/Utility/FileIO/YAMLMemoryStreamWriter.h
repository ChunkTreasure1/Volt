#pragma once

#include "Volt/Utility/FileIO/YAMLStreamWriter.h"
#include "Volt/Core/Buffer.h"

namespace Volt
{
	class YAMLMemoryStreamWriter : public YAMLStreamWriter
	{
	public:
		~YAMLMemoryStreamWriter() = default;

		Buffer WriteAndGetBuffer() const;
	};
}
