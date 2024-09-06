#pragma once

#include "CoreUtilities/FileIO/YAMLStreamReader.h"

class Buffer;

class VTCOREUTIL_API YAMLMemoryStreamReader : public YAMLStreamReader
{
public:
	~YAMLMemoryStreamReader() override = default;

	const bool ReadBuffer(const Buffer& buffer);
	const bool ConsumeBuffer(Buffer& buffer);
};
