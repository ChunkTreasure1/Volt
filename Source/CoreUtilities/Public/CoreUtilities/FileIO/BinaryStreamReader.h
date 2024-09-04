#pragma once

#include "CoreUtilities/FileIO/StreamCommon.h"
#include "CoreUtilities/Buffer/Buffer.h"
#include "CoreUtilities/Containers/Vector.h"

#include <fstream>

#include <array>
#include <string>
#include <map>
#include <unordered_map>

class VTCOREUTIL_API BinaryStreamReader
{
public:
	BinaryStreamReader(const std::filesystem::path& filePath);
	BinaryStreamReader(const std::filesystem::path& filePath, const size_t maxLoadSize);

	bool IsStreamValid() const;

	template<typename T>
	void Read(T& outData);

	template<>
	void Read(std::string& data);

	template<>
	void Read(Buffer& data);

	template<typename T>
	bool TryRead(T& outData);

	template<typename F>
	void Read(Vector<F>& data);

	template<typename F>
	void ReadRaw(Vector<F>& data);

	template<typename F, size_t COUNT>
	void Read(std::array<F, COUNT>& data);

	template<typename Key, typename Value>
	void Read(std::map<Key, Value>& data);

	template<typename Key, typename Value>
	void Read(std::unordered_map<Key, Value>& data);

	void Read(void* data);

	void ResetHead();
	TypeHeader ReadTypeHeader();

private:
	void ReadData(void* outData, const TypeHeader& serializedTypeHeader, const TypeHeader& constructedTypeHeader);

	bool Decompress(size_t compressedDataOffset);

	Vector<uint8_t> m_data;
	size_t m_currentOffset = 0;
	bool m_streamValid = false;
	bool m_compressed = false;
};

template<typename T>
inline void BinaryStreamReader::Read(T& outData)
{
	constexpr size_t typeSize = sizeof(T);

	TypeHeader typeHeader{};
	typeHeader.totalTypeSize = static_cast<uint32_t>(typeSize);

	TypeHeader serializedTypeHeader = ReadTypeHeader();

	if constexpr (std::is_trivial_v<T>)
	{
		ReadData(&outData, serializedTypeHeader, typeHeader);
	}
	else
	{
		T::Deserialize(*this, outData);
	}
}

template<typename T>
inline bool BinaryStreamReader::TryRead(T& outData)
{
	constexpr size_t typeSize = sizeof(T);

	TypeHeader typeHeader{};
	typeHeader.totalTypeSize = static_cast<uint32_t>(typeSize);

	TypeHeader serializedTypeHeader = ReadTypeHeader();

	if (serializedTypeHeader.totalTypeSize != typeHeader.totalTypeSize)
	{
		return false;
	}

	if constexpr (std::is_trivial_v<T>)
	{
		ReadData(&outData, serializedTypeHeader, typeHeader);
	}
	else
	{
		T::Deserialize(*this, outData);
	}

	return true;
}

template<>
inline void BinaryStreamReader::Read(std::string& data)
{
	TypeHeader typeHeader{};
	TypeHeader serializedTypeHeader = ReadTypeHeader();

	data.resize(serializedTypeHeader.totalTypeSize);
	if (serializedTypeHeader.totalTypeSize > 0)
	{
		ReadData(data.data(), serializedTypeHeader, typeHeader);
	}
}

template<>
inline void BinaryStreamReader::Read(Buffer& data)
{
	TypeHeader typeHeader{};
	TypeHeader serializedTypeHeader = ReadTypeHeader();

	data.Resize(serializedTypeHeader.totalTypeSize);
	if (serializedTypeHeader.totalTypeSize > 0)
	{
		ReadData(data.As<void>(), serializedTypeHeader, typeHeader);
	}
}

template<typename F>
inline void BinaryStreamReader::Read(Vector<F>& data)
{
	TypeHeader typeHeader{};
	TypeHeader serializedTypeHeader = ReadTypeHeader();

	data.resize(serializedTypeHeader.totalTypeSize);

	if constexpr (std::is_trivial_v<F>)
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
inline void BinaryStreamReader::ReadRaw(Vector<F>& data)
{
	TypeHeader typeHeader{};
	TypeHeader serializedTypeHeader = ReadTypeHeader();

	data.resize(serializedTypeHeader.totalTypeSize);
	serializedTypeHeader.totalTypeSize *= sizeof(F);
	ReadData(data.data(), serializedTypeHeader, typeHeader);
}

template<typename F, size_t COUNT>
inline void BinaryStreamReader::Read(std::array<F, COUNT>& data)
{
	TypeHeader typeHeader{};
	TypeHeader serializedTypeHeader = ReadTypeHeader();

	if constexpr (std::is_trivial_v<F>)
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
	TypeHeader serializedTypeHeader = ReadTypeHeader();

	const size_t elementCount = serializedTypeHeader.totalTypeSize;

	for (size_t i = 0; i < elementCount; i++)
	{
		Key key{};

		if constexpr (std::is_trivial_v<Key>)
		{
			TypeHeader keyTypeHeader{};
			keyTypeHeader.totalTypeSize = sizeof(Value);

			ReadData(&key, keyTypeHeader, keyTypeHeader);
		}
		else
		{
			Key::Deserialize(*this, key);
		}

		Value value{};

		if constexpr (std::is_trivial_v<Value>)
		{
			TypeHeader valueTypeHeader{};
			valueTypeHeader.totalTypeSize = sizeof(Value);

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
	TypeHeader serializedTypeHeader = ReadTypeHeader();

	const size_t elementCount = serializedTypeHeader.totalTypeSize;

	for (size_t i = 0; i < elementCount; i++)
	{
		Key key{};

		if constexpr (std::is_trivial_v<Key>)
		{
			TypeHeader keyTypeHeader{};
			ReadData(&key, keyTypeHeader, keyTypeHeader);
		}
		else if constexpr (std::is_same<Key, std::string>::value)
		{
			Read(key);
		}
		else
		{
			Key::Deserialize(*this, key);
		}
		 
		Value value{};

		if constexpr (std::is_trivial_v<Value>)
		{
			TypeHeader valueTypeHeader{};
			valueTypeHeader.totalTypeSize = sizeof(Value);

			ReadData(&value, valueTypeHeader, valueTypeHeader);
		}
		else if constexpr (std::is_same<Value, std::string>::value)
		{
			Read(value);
		}
		else
		{
			Value::Deserialize(*this, value);
		}

		data[key] = value;
	}
}
