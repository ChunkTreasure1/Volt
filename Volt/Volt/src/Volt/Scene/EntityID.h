#pragma once

namespace Volt
{
	class EntityID
	{
	public:
		EntityID();
		constexpr EntityID(uint32_t uuid)
			: m_uuid(uuid)
		{
		}

		EntityID(const EntityID&) = default;
		~EntityID() = default;

		operator uint32_t() const { return m_uuid; }
	private:
		uint32_t m_uuid;
	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<Volt::EntityID>
	{
		std::size_t operator()(const Volt::EntityID& uuid) const
		{
			return (uint32_t)uuid;
		}
	};
}
