#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <unordered_map>

namespace Volt
{
	class BinaryStreamWriter
	{
	public:
		template<typename T>
		[[nodiscard]] const size_t GetBinarySizeOfType(const T& object) const;

		template<> [[nodiscard]] const size_t GetBinarySizeOfType(const std::string& object) const;
		template<typename F> [[nodiscard]] const size_t GetBinarySizeOfType(const std::vector<F>& object) const;
		template<typename F, size_t COUNT> [[nodiscard]] const size_t GetBinarySizeOfType(const std::array<F, COUNT>& object) const;
		template<typename Key, typename Value> [[nodiscard]] const size_t GetBinarySizeOfType(const std::map<Key, Value>& object) const;
		template<typename Key, typename Value> [[nodiscard]] const size_t GetBinarySizeOfType(const std::unordered_map<Key, Value>& object) const;

		template<typename T>
		void Write(const T& data);

		template<>
		void Write(const std::string& data);

		template<typename F>
		void Write(const std::vector<F>& data);

		template<typename F, size_t COUNT>
		void Write(const std::array<F, COUNT>& data);

		template<typename Key, typename Value>
		void Write(const std::map<Key, Value>& data);

		template<typename Key, typename Value>
		void Write(const std::unordered_map<Key, Value>& data);

		[[nodiscard]] const size_t GetSize() const { return m_data.size(); }

	private:
		struct TypeHeader
		{
			uint16_t baseTypeSize;
			uint32_t totalTypeSize;
		};

		void WriteData(const void* data, const size_t size, const TypeHeader& typeHeader);
		void WriteData(const void* data, const size_t size);

		std::vector<uint8_t> m_data;
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
	inline const size_t BinaryStreamWriter::GetBinarySizeOfType(const std::vector<F>& object) const
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
	inline void BinaryStreamWriter::Write(const T& data)
	{
		constexpr size_t typeSize = sizeof(T);

		if constexpr (std::is_trivial<T>())
		{
			TypeHeader header{};
			header.baseTypeSize = typeSize;
			header.totalTypeSize = typeSize;

			WriteData(&data, typeSize, header);
		}
		else
		{
			T::Serialize(*this, data);
		}
	}

	template<>
	inline void BinaryStreamWriter::Write(const std::string& data)
	{
		TypeHeader header{};
		header.baseTypeSize = sizeof(std::string);
		header.totalTypeSize = static_cast<uint32_t>(data.size());

		WriteData(data.data(), data.size(), header);
	}

	template<typename F>
	inline void BinaryStreamWriter::Write(const std::vector<F>& data)
	{
		TypeHeader header{};
		header.baseTypeSize = sizeof(std::vector<F>);
		header.totalTypeSize = static_cast<uint32_t>(data.size() * sizeof(F));

		if constexpr (std::is_trivial_v<F>)
		{
			WriteData(data.data(), data.size() * sizeof(F), header);
		}
		else
		{
			for (const auto& obj : data)
			{
				F::Serialize(*this, obj);
			}
		}
	}

	template<typename F, size_t COUNT>
	inline void BinaryStreamWriter::Write(const std::array<F, COUNT>& data)
	{
		TypeHeader header{};
		header.baseTypeSize = sizeof(std::array<F, COUNT>);
		header.totalTypeSize = COUNT * sizeof(F);

		if constexpr (std::is_trivial_v<F>)
		{
			WriteData(data.data(), data.size() * sizeof(F), header);
		}
		else
		{
			for (const auto& obj : data)
			{
				F::Serialize(*this, obj);
			}
		}
	}

	template<typename Key, typename Value>
	inline void BinaryStreamWriter::Write(const std::map<Key, Value>& data)
	{
		TypeHeader header{};
		header.baseTypeSize = sizeof(std::map<Key, Value>& data);
		header.totalTypeSize = data.size() * sizeof(Key) + data.size() * sizeof(Value);

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


	}

	template<typename Key, typename Value>
	inline void BinaryStreamWriter::Write(const std::unordered_map<Key, Value>& data)
	{
	}
}
