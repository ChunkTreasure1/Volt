#pragma once
#include <cstddef>
#include <stdint.h>

namespace Volt
{
	class BinaryStreamWriter;

	class UUID
	{
	public:
		UUID();
		constexpr UUID(uint64_t uuid)
			: myUUID(uuid)
		{}

		UUID(const UUID&) = default;
		~UUID() = default;

		operator uint64_t() const { return myUUID; }

		static void Serialize(BinaryStreamWriter& streamWriter, const UUID& data);

	private:
		uint64_t myUUID;
	};

	class UUID32
	{
	public:
		UUID32();
		constexpr UUID32(uint32_t uuid)
			: m_uuid(uuid)
		{}

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
