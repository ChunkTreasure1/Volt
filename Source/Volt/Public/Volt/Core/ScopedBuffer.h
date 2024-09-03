#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include <fstream>
#include <filesystem>

namespace Volt
{
	class ScopedBuffer
	{
	public:
		inline ScopedBuffer(size_t aSize);
		inline ScopedBuffer() = default;
		inline ~ScopedBuffer();

		inline void Release();
		inline void Allocate(size_t aSize);
		inline void Resize(size_t aSize);
		inline void Copy(const void* aSrcData, size_t aSize, size_t aOffset = 0);

		inline const bool IsValid() const;
		inline const size_t GetSize() const;

		inline static bool WriteToFile(ScopedBuffer buffer, const std::filesystem::path& targetPath);
		inline static ScopedBuffer ReadFromFile(const std::filesystem::path& targetPath);

		template<typename T>
		inline T* As(size_t offset = 0) const;

	private:
		uint8_t* myData = nullptr;
		size_t mySize = 0;
	};

	inline ScopedBuffer::ScopedBuffer(size_t aSize)
	{
		Allocate(aSize);
	}

	inline ScopedBuffer::~ScopedBuffer()
	{
		Release();
	}

	inline void ScopedBuffer::Release()
	{
		if (myData)
		{
			delete[] myData;
			myData = nullptr;
		}
	}

	inline void ScopedBuffer::Allocate(size_t aSize)
	{
		Release();
		myData = new uint8_t[aSize];
		mySize = aSize;
	}

	inline void ScopedBuffer::Resize(size_t aSize)
	{
		if (mySize < aSize)
		{
			uint8_t* newBuffer = new uint8_t[aSize];

			if (myData)
			{
				memcpy_s(newBuffer, aSize, myData, mySize);
				delete[] myData;
			}

			mySize = aSize;
			myData = newBuffer;
		}
	}

	inline void ScopedBuffer::Copy(const void* aSrcData, size_t aSize, size_t aOffset)
	{
		VT_ASSERT_MSG(aOffset + aSize <= mySize, "Cannot copy into buffer of lesser size!");
		memcpy_s(myData + aOffset, mySize, aSrcData, aSize);
	}

	inline const bool ScopedBuffer::IsValid() const
	{
		return myData != nullptr;
	}

	inline const size_t ScopedBuffer::GetSize() const
	{
		return mySize;
	}

	inline bool ScopedBuffer::WriteToFile(ScopedBuffer buffer, const std::filesystem::path& targetPath)
	{
		std::ofstream file(targetPath, std::ios::out | std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}

		file.write(reinterpret_cast<char*>(buffer.myData), buffer.mySize);
		file.close();

		return true;
	}

	inline ScopedBuffer ScopedBuffer::ReadFromFile(const std::filesystem::path& targetPath)
	{
		if (!std::filesystem::exists(targetPath))
		{
			return {};
		}

		std::ifstream file(targetPath, std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			return {};
		}

		Vector<uint8_t> totalData;
		const size_t srcSize = file.seekg(0, std::ios::end).tellg();
		totalData.resize(srcSize);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		file.close();

		ScopedBuffer buffer{ srcSize };
		buffer.Copy(totalData.data(), totalData.size());

		return buffer;
	}

	template<typename T>
	inline T* ScopedBuffer::As(size_t offset) const
	{
		return reinterpret_cast<T*>(myData + offset);
	}
}
