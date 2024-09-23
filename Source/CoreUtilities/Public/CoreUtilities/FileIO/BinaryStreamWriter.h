#pragma once

#include "CoreUtilities/FileIO/StreamCommon.h"
#include "CoreUtilities/Buffer/Buffer.h"
#include "CoreUtilities/Containers/Vector.h"

#include "CoreUtilities/Containers/Map.h"

#include <array>
#include <string>
#include <map>
#include <unordered_map>

class VTCOREUTIL_API BinaryStreamWriter
{
public:
	void WriteToDisk(const std::filesystem::path& targetFilepath, bool compress, size_t compressedDataOffset);
	[[nodiscard]] const size_t GetSize() const { return m_data.size(); }

	template<typename T>
	[[nodiscard]] const size_t GetBinarySizeOfType(const T& object) const;

	template<> [[nodiscard]] const size_t GetBinarySizeOfType(const std::string& object) const;
	template<typename F> [[nodiscard]] const size_t GetBinarySizeOfType(const Vector<F>& object) const;
	template<typename F, size_t COUNT> [[nodiscard]] const size_t GetBinarySizeOfType(const std::array<F, COUNT>& object) const;
	template<typename Key, typename Value> [[nodiscard]] const size_t GetBinarySizeOfType(const std::map<Key, Value>& object) const;
	template<typename Key, typename Value> [[nodiscard]] const size_t GetBinarySizeOfType(const std::unordered_map<Key, Value>& object) const;

	template<typename T>
	size_t Write(const T& data);

	template<>
	size_t Write(const std::string& data);

	template<>
	size_t Write(const Buffer& buffer);

	template<typename F>
	size_t Write(const Vector<F>& data);

	template<typename F>
	size_t WriteRaw(const Vector<F>& data);

	template<typename F, size_t COUNT>
	size_t Write(const std::array<F, COUNT>& data);

	template<typename Key, typename Value>
	size_t Write(const std::map<Key, Value>& data);

	template<typename Key, typename Value>
	size_t Write(const std::unordered_map<Key, Value>& data);

	template<typename Key, typename Value>
	size_t Write(const vt::map<Key, Value>& data);

	size_t Write(const void* data, const size_t size);

private:
	bool GetCompressed(Vector<uint8_t>& result, size_t compressedDataOffset = 0);

	void WriteTypeHeader(const TypeHeader& typeHeader);
	void WriteData(const void* data, const size_t size);

	Vector<uint8_t> m_data;
};

template<typename T>
inline const size_t BinaryStreamWriter::GetBinarySizeOfType(const T& object) const
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);

	if constexpr (std::is_trivial_v<T>)
	{
		return sizeof(T) + typeHeaderSize;
	}

	BinaryStreamWriter tempSerializer{};
	T::Serialize(tempSerializer, object);

	return tempSerializer.GetSize() + typeHeaderSize;
}

template<>
inline const size_t BinaryStreamWriter::GetBinarySizeOfType(const std::string& object) const
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);
	return object.size() + typeHeaderSize;
}

template<typename F>
inline const size_t BinaryStreamWriter::GetBinarySizeOfType(const Vector<F>& object) const
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);
	return object.size() * sizeof(F) + typeHeaderSize;
}

template<typename F, size_t COUNT>
inline const size_t BinaryStreamWriter::GetBinarySizeOfType(const std::array<F, COUNT>& object) const
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);
	return COUNT * sizeof(F) + typeHeaderSize;
}

template<typename Key, typename Value>
inline const size_t BinaryStreamWriter::GetBinarySizeOfType(const std::map<Key, Value>& object) const
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);
	return object.size() * sizeof(Key) + object.size() * sizeof(Value) + typeHeaderSize;
}

template<typename Key, typename Value>
inline const size_t BinaryStreamWriter::GetBinarySizeOfType(const std::unordered_map<Key, Value>& object) const
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);
	return object.size() * sizeof(Key) + object.size() * sizeof(Value) + typeHeaderSize;
}

template<typename T>
inline size_t BinaryStreamWriter::Write(const T& data)
{
	constexpr size_t typeSize = sizeof(T);

	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(typeSize);

	WriteTypeHeader(header);

	if constexpr (std::is_trivial_v<T>)
	{
		WriteData(&data, typeSize);
	}
	else
	{
		T::Serialize(*this, data);
	}

	return m_data.size();
}

template<>
inline size_t BinaryStreamWriter::Write(const std::string& data)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(data.size());

	WriteTypeHeader(header);
	if (!data.empty())
	{
		WriteData(data.data(), data.size());
	}

	return m_data.size();
}

template<>
inline size_t BinaryStreamWriter::Write(const Buffer& buffer)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(buffer.GetSize());

	WriteTypeHeader(header);
	if (buffer.GetSize() > 0)
	{
		WriteData(buffer.As<void>(), buffer.GetSize());
	}

	return m_data.size();
}

template<typename F>
inline size_t BinaryStreamWriter::Write(const Vector<F>& data)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(data.size());

	WriteTypeHeader(header);

	if constexpr (std::is_trivial_v<F>)
	{
		if (!data.empty())
		{
			WriteData(data.data(), data.size() * sizeof(F));
		}
	}
	else
	{
		for (const auto& obj : data)
		{
			Write(obj);
		}
	}

	return m_data.size();
}

template<typename F>
inline size_t BinaryStreamWriter::WriteRaw(const Vector<F>& data)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(data.size());

	WriteTypeHeader(header);
	if (!data.empty())
	{
		WriteData(data.data(), data.size() * sizeof(F));
	}

	return m_data.size();
}

template<typename F, size_t COUNT>
inline size_t BinaryStreamWriter::Write(const std::array<F, COUNT>& data)
{
	static_assert(COUNT != 0);

	TypeHeader header{};
	header.totalTypeSize = COUNT;

	WriteTypeHeader(header);

	if constexpr (std::is_trivial_v<F>)
	{
		WriteData(data.data(), COUNT * sizeof(F));
	}
	else
	{
		for (const auto& obj : data)
		{
			Write(obj);
		}
	}

	return m_data.size();
}

template<typename Key, typename Value>
inline size_t BinaryStreamWriter::Write(const std::map<Key, Value>& data)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(data.size());

	WriteTypeHeader(header);

	for (const auto& [key, value] : data)
	{
		if constexpr (std::is_trivial_v<Key>)
		{
			WriteData(&key, sizeof(Key));
		}
		else
		{
			Key::Serialize(*this, key);
		}

		if constexpr (std::is_trivial_v<Value>)
		{
			WriteData(&value, sizeof(Value));
		}
		else
		{
			Value::Serialize(*this, value);
		}
	}

	return m_data.size();
}

template<typename Key, typename Value>
inline size_t BinaryStreamWriter::Write(const std::unordered_map<Key, Value>& data)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(data.size());

	WriteData(&header, sizeof(header));

	for (const auto& [key, value] : data)
	{
		if constexpr (std::is_trivial_v<Key>)
		{
			WriteData(&key, sizeof(Key));
		}
		else if constexpr (std::is_same<Key, std::string>::value)
		{
			Write(key);
		}
		else
		{
			Key::Serialize(*this, key);
		}

		if constexpr (std::is_trivial_v<Value>)
		{
			WriteData(&value, sizeof(Value));
		}
		else if constexpr (std::is_same<Value, std::string>::value)
		{
			Write(value);
		}
		else
		{
			Value::Serialize(*this, value);
		}
	}

	return m_data.size();
}

template<typename Key, typename Value>
inline size_t BinaryStreamWriter::Write(const vt::map<Key, Value>& data)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(data.size());

	WriteData(&header, sizeof(header));

	for (const auto& [key, value] : data)
	{
		if constexpr (std::is_trivial_v<Key>)
		{
			WriteData(&key, sizeof(Key));
		}
		else if constexpr (std::is_same<Key, std::string>::value)
		{
			Write(key);
		}
		else
		{
			Key::Serialize(*this, key);
		}

		if constexpr (std::is_trivial_v<Value>)
		{
			WriteData(&value, sizeof(Value));
		}
		else if constexpr (std::is_same<Value, std::string>::value)
		{
			Write(value);
		}
		else
		{
			Value::Serialize(*this, value);
		}
	}

	return m_data.size();
}
