#pragma once

#include "Volt/Core/Buffer.h"
#include "Volt/Utility/FileIO/StreamCommon.h"

#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <map>
#include <unordered_map>

namespace Volt
{
	class BinaryStreamReader
	{
	public:
		BinaryStreamReader(const std::filesystem::path& filePath);

		bool IsStreamValid() const;

		template<typename T>
		void Read(T& outData);

		template<>
		void Read(std::string& data);

		template<>
		void Read(Buffer& data);

		template<typename F>
		void Read(std::vector<F>& data);

		template<typename F>
		void ReadRaw(std::vector<F>& data);
		
		template<typename F, size_t COUNT>
		void Read(std::array<F, COUNT>& data);

		template<typename Key, typename Value>
		void Read(std::map<Key, Value>& data);

		template<typename Key, typename Value>
		void Read(std::unordered_map<Key, Value>& data);

		void Read(void* data);

	private:
		TypeHeader ReadTypeHeader();
		void ReadData(void* outData, const TypeHeader& serializedTypeHeader, const TypeHeader& constructedTypeHeader);

		bool Decompress(size_t compressedDataOffset);

		std::vector<uint8_t> m_data;
		size_t m_currentOffset = 0;
		bool m_streamValid = false;
	};

	template<typename T>
	inline void BinaryStreamReader::Read(T& outData)
	{
		constexpr size_t typeSize = sizeof(T);

		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = static_cast<uint16_t>(typeSize);
		typeHeader.totalTypeSize = static_cast<uint32_t>(typeSize);

		TypeHeader serializedTypeHeader = ReadTypeHeader();

		if constexpr (std::is_trivial<T>())
		{
			ReadData(&outData, serializedTypeHeader, typeHeader);
		}
		else
		{
			VT_CORE_ASSERT(serializedTypeHeader.baseTypeSize == typeHeader.baseTypeSize, "Base Type sizes must match!");
			T::Deserialize(*this, outData);
		}
	}

	template<>
	inline void BinaryStreamReader::Read(std::string& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::string);
		TypeHeader serializedTypeHeader = ReadTypeHeader();
	
		data.resize(serializedTypeHeader.totalTypeSize);
		ReadData(data.data(), serializedTypeHeader, typeHeader);
	}

	template<>
	inline void BinaryStreamReader::Read(Buffer& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(Buffer);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		data.Resize(serializedTypeHeader.totalTypeSize);
		ReadData(data.As<void>(), serializedTypeHeader, typeHeader);
	}

	template<typename F>
	inline void BinaryStreamReader::Read(std::vector<F>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::vector<F>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		data.resize(serializedTypeHeader.totalTypeSize);

		if constexpr (std::is_trivial<F>())
		{
			// We must multiply type size to get correct byte size
			serializedTypeHeader.totalTypeSize *= sizeof(F);
			ReadData(data.data(), serializedTypeHeader, typeHeader);
		}
		else
		{
			for (size_t i = 0; i < data.size(); i++)
			{
				Read(data[i]);
			}
		}
	}

	template<typename F>
	inline void BinaryStreamReader::ReadRaw(std::vector<F>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::vector<F>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		data.resize(serializedTypeHeader.totalTypeSize);
		serializedTypeHeader.totalTypeSize *= sizeof(F);
		ReadData(data.data(), serializedTypeHeader, typeHeader);
	}

	template<typename F, size_t COUNT>
	inline void BinaryStreamReader::Read(std::array<F, COUNT>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::array<F, COUNT>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		if constexpr (std::is_trivial<F>())
		{
			// We must multiply type size to get correct byte size
			serializedTypeHeader.totalTypeSize *= sizeof(F);
			ReadData(data.data(), serializedTypeHeader, typeHeader);
		}
		else
		{
			for (size_t i = 0; i < data.size(); i++)
			{
				Read(data[i]);
			}
		}
	}

	template<typename Key, typename Value>
	inline void BinaryStreamReader::Read(std::map<Key, Value>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::map<Key, Value>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		const size_t elementCount = serializedTypeHeader.totalTypeSize;

		for (size_t i = 0; i < elementCount; i++)
		{
			Key key{};

			if constexpr (std::is_trivial<Key>())
			{
				TypeHeader keyTypeHeader{};
				keyTypeHeader.baseTypeSize = sizeof(Key);

				TypeHeader keySerializedTypeHeader = ReadTypeHeader();
				ReadData(&key, keySerializedTypeHeader, keyTypeHeader);
			}
			else
			{
				Key::Deserialize(*this, key);
			}

			Value value{};

			if constexpr (std::is_trivial<Value>())
			{
				TypeHeader valueTypeHeader{};
				valueTypeHeader.baseTypeSize = sizeof(Value);

				TypeHeader valueSerializedTypeHeader = ReadTypeHeader();
				ReadData(&value, valueSerializedTypeHeader, valueTypeHeader);
			}
			else
			{
				Value::Deserialize(*this, value);
			}

			data.emplace(key, value);
		}
	}

	template<typename Key, typename Value>
	inline void BinaryStreamReader::Read(std::unordered_map<Key, Value>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::unordered_map<Key, Value>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		const size_t elementCount = serializedTypeHeader.totalTypeSize;

		for (size_t i = 0; i < elementCount; i++)
		{
			Key key{};

			if constexpr (std::is_trivial<Key>())
			{
				TypeHeader keyTypeHeader{};
				keyTypeHeader.baseTypeSize = sizeof(Key);

				TypeHeader keySerializedTypeHeader = ReadTypeHeader();
				ReadData(&key, keySerializedTypeHeader, keyTypeHeader);
			}
			else
			{
				Key::Deserialize(*this, key);
			}

			Value value{};

			if constexpr (std::is_trivial<Value>())
			{
				TypeHeader valueTypeHeader{};
				valueTypeHeader.baseTypeSize = sizeof(Value);

				TypeHeader valueSerializedTypeHeader = ReadTypeHeader();
				ReadData(&value, valueSerializedTypeHeader, valueTypeHeader);
			}
			else
			{
				Value::Deserialize(*this, value);
			}

			data.emplace(key, value);
		}
	}
}