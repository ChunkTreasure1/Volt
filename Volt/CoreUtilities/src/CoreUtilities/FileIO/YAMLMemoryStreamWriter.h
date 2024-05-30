#pragma once

#include "CoreUtilities/FileIO/YAMLStreamWriter.h"
#include "CoreUtilities/Buffer/Buffer.h"

class VTCOREUTIL_API YAMLMemoryStreamWriter : public YAMLStreamWriter
{
public:
	~YAMLMemoryStreamWriter() = default;

	Buffer WriteAndGetBuffer() const;
};
