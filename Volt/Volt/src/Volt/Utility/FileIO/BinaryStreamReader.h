#pragma once

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

	private:
		TypeHeader ReadTypeHeader();
		void ReadData(void* outData, const TypeHeader& serializedTypeHeader, const TypeHeader& constructedTypeHeader);

		std::ifstream m_stream{};
	};

	template<typename T>
	inline void BinaryStreamReader::Read(T& outData)
	{
		if constexpr (std::is_trivial<T>())
		{
			constexpr size_t typeSize = sizeof(T);

			TypeHeader typeHeader{};
			typeHeader.baseTypeSize = static_cast<uint16_t>(typeSize);
			typeHeader.totalTypeSize = static_cast<uint32_t>(typeSize);
		
			TypeHeader serializedTypeHeader = ReadTypeHeader();
			ReadData(&outData, serializedTypeHeader, typeHeader);
		}
		else
		{
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

	template<typename F>
	inline void BinaryStreamReader::Read(std::vector<F>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::vector<F>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		data.resize(serializedTypeHeader.totalTypeSize / sizeof(F));

		if constexpr (std::is_trivial<F>())
		{
			ReadData(data.data(), serializedTypeHeader, typeHeader);
		}
		else
		{
			for (size_t i = 0; i < data.size(); i++)
			{
				F::Deserialize(*this, data[i]);
			}
		}
	}

	template<typename F>
	inline void BinaryStreamReader::ReadRaw(std::vector<F>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::vector<F>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		data.resize(serializedTypeHeader.totalTypeSize / sizeof(F));
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
			ReadData(data.data(), serializedTypeHeader, typeHeader);
		}
		else
		{
			for (size_t i = 0; i < data.size(); i++)
			{
				F::Deserialize(*this, data[i]);
			}
		}
	}

	template<typename Key, typename Value>
	inline void BinaryStreamReader::Read(std::map<Key, Value>& data)
	{
		TypeHeader typeHeader{};
		typeHeader.baseTypeSize = sizeof(std::map<Key, Value>);
		TypeHeader serializedTypeHeader = ReadTypeHeader();

		const size_t elementCount = serializedTypeHeader.totalTypeSize / (sizeof(Key) + sizeof(Value));

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

		const size_t elementCount = serializedTypeHeader.totalTypeSize / (sizeof(Key) + sizeof(Value));

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
