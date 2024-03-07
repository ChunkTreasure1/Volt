#pragma once

#include "Volt/Log/Log.h"
#include "Volt/Core/Base.h"

#include <fstream>

namespace Volt
{
	class Buffer
	{
	public:
		inline Buffer(size_t aSize);
		inline Buffer() = default;
		inline ~Buffer();

		inline void Release();
		inline void Allocate(size_t aSize);
		inline void Clear();
		inline void Resize(size_t aSize);
		inline void Copy(const void* aSrcData, size_t aSize, size_t aOffset = 0);

		inline const bool IsValid() const;
		inline const size_t GetSize() const;

		inline static bool WriteToFile(Buffer buffer, const std::filesystem::path& targetPath);
		inline static Buffer ReadFromFile(const std::filesystem::path& targetPath);

		template<typename T>
		inline T* As(size_t offset = 0) const;

	private:
		uint8_t* myData = nullptr;
		size_t mySize = 0;
	};

	inline Buffer::Buffer(size_t aSize)
	{
		Allocate(aSize);
	}

	inline Buffer::~Buffer()
	{
	}

	inline void Buffer::Release()
	{
		if (myData)
		{
			delete[] myData;
			myData = nullptr;
		}
	}

	inline void Buffer::Allocate(size_t aSize)
	{
		if (aSize == 0)
		{
			return;
		}

		Release();
		myData = new uint8_t[aSize];
		memset(myData, 0, aSize);

		mySize = aSize;
	}

	inline void Buffer::Clear()
	{
		memset(myData, 0, mySize);
	}

	inline void Buffer::Resize(size_t aSize)
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

	inline void Buffer::Copy(const void* aSrcData, size_t aSize, size_t aOffset)
	{
		if (aSize == 0)
		{
			return;
		}

		VT_CORE_ASSERT(aOffset + aSize <= mySize, "Cannot copy into buffer of lesser size!");
		memcpy_s(myData + aOffset, mySize, aSrcData, aSize);
	}

	inline const bool Buffer::IsValid() const
	{
		return myData != nullptr;
	}

	inline const size_t Buffer::GetSize() const
	{
		return mySize;
	}

	inline bool Buffer::WriteToFile(Buffer buffer, const std::filesystem::path& targetPath)
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

	inline Buffer Buffer::ReadFromFile(const std::filesystem::path& targetPath)
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

		std::vector<uint8_t> totalData;
		const size_t srcSize = file.seekg(0, std::ios::end).tellg();
		totalData.resize(srcSize);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		file.close();

		Buffer buffer{ srcSize };
		buffer.Copy(totalData.data(), totalData.size());

		return buffer;
	}

	template<typename T>
	inline T* Buffer::As(size_t offset) const
	{
		return reinterpret_cast<T*>(myData + offset);
	}
}
