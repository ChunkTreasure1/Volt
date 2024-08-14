#pragma once

#include "CoreUtilities/Core.h"

#include <cstddef>
#include <stdint.h>
#include <format>

class BinaryStreamReader;
class BinaryStreamWriter;

class VTCOREUTIL_API UUID64
{
public:
	UUID64();
	UUID64(uint64_t uuid);

	UUID64(const UUID64&) = default;
	~UUID64() = default;

	static void Serialize(BinaryStreamWriter& streamWriter, const UUID64& data);
	static void Deserialize(BinaryStreamReader& streamReader, UUID64& outData);

	operator uint64_t() const;

	VT_NODISCARD VT_INLINE const uint64_t Get() const { return m_uuid; }
	
private:
	uint64_t m_uuid;
};

class VTCOREUTIL_API UUID32
{
public:
	UUID32();
	UUID32(uint32_t uuid);

	UUID32(const UUID32&) = default;
	~UUID32() = default;

	static void Serialize(BinaryStreamWriter& streamWriter, const UUID32& data);
	static void Deserialize(BinaryStreamReader& streamReader, UUID32& outData);

	operator uint32_t() const;

	VT_NODISCARD VT_INLINE const uint32_t Get() const { return m_uuid; }
private:
	uint32_t m_uuid;
};

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<UUID64>
	{
		std::size_t operator()(const UUID64& uuid) const
		{
			return static_cast<size_t>(uuid);
		}
	};

	template<>
	struct hash<UUID32>
	{
		std::size_t operator()(const UUID32& uuid) const
		{
			return static_cast<size_t>(uuid);
		}
	};

	template <>
	struct formatter<UUID32> : formatter<string>
	{
		auto format(UUID32 id, format_context& ctx) const
		{
			return formatter<string>::format(
			  std::format("{}", id.Get()), ctx);
		}
	};

	template <>
	struct formatter<UUID64> : formatter<string>
	{
		auto format(UUID64 id, format_context& ctx) const
		{
			return formatter<string>::format(
			  std::format("{}", id.Get()), ctx);
		}
	};
}
