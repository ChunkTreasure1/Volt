#pragma once
#include <cstddef>
#include <stdint.h>

	class UUID64
	{
	public:
		UUID64();
		UUID64(uint64_t uuid);

		UUID64(const UUID64&) = default;
		~UUID64() = default;

		operator uint64_t() const { return m_uuid; }
	private:
		uint64_t m_uuid;
	};

	class UUID32
	{
	public:
		UUID32();
		UUID32(uint32_t uuid);

		UUID32(const UUID32&) = default;
		~UUID32() = default;

		operator uint32_t() const { return m_uuid; }
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
			return (uint64_t)uuid;
		}
	};

	template<>
	struct hash<UUID32>
	{
		std::size_t operator()(const UUID32& uuid) const
		{
			return (uint32_t)uuid;
		}
	};
}