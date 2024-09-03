#include "vtpch.h"
#include "Volt/Core/BinarySerializer.h"

namespace Volt
{
	BinarySerializer::BinarySerializer(const std::filesystem::path& targetFile, const size_t maxSize)
		: myTargetPath(targetFile), myMaxSize(maxSize)
	{
		if (myMaxSize > 0)
		{
			myDataBuffer.Allocate(myMaxSize);
		}
	}

	BinarySerializer::~BinarySerializer()
	{
		myDataBuffer.Release();
	}

	void BinarySerializer::Serialize(const void* data, const size_t size)
	{
		if (myMaxSize > 0)
		{
			assert(myCurrentSize + size <= myMaxSize && "Total size is larger than the set max size!");
		}
		else
		{
			if (myDataBuffer.GetSize() < myCurrentSize + size)
			{
				myDataBuffer.Resize(myCurrentSize + size);
			}
		}

		myDataBuffer.Copy(data, size, myCurrentSize);
		myCurrentSize += size;
	}

	void BinarySerializer::WriteToFile()
	{
		std::ofstream output(myTargetPath, std::ios::binary);
		output.write(myDataBuffer.As<char>(), myDataBuffer.GetSize());
		output.close();
	}
}