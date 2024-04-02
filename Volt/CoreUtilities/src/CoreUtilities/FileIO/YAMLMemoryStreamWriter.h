#pragma once

#include "CoreUtilities/FileIO/YAMLStreamWriter.h"
#include "CoreUtilities/Buffer/Buffer.h"

class YAMLMemoryStreamWriter : public YAMLStreamWriter
{
public:
	~YAMLMemoryStreamWriter() = default;

	Buffer WriteAndGetBuffer() const;
};
