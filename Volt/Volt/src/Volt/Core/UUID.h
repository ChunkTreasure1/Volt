#pragma once
#include <cstddef>
#include <stdint.h>

namespace Volt
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);

		UUID(const UUID&) = default;
		~UUID() = default;

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
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<Volt::UUID>
	{
		std::size_t operator()(const Volt::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

	template<>
	struct hash<Volt::UUID32>
	{
		std::size_t operator()(const Volt::UUID32& uuid) const
		{
			return (uint32_t)uuid;
		}
	};
}
