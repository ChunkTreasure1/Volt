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

		operator uint64_t() const { return myUUID; }
	private:
		uint64_t myUUID;
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
}