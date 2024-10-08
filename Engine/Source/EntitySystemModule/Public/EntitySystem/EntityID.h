#pragma once

#include "EntitySystem/Config.h"

class BinaryStreamReader;
class BinaryStreamWriter;

namespace Volt
{
	class VTES_API EntityID
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

		static void Serialize(BinaryStreamWriter& streamWriter, const EntityID& data);
		static void Deserialize(BinaryStreamReader& streamReader, EntityID& outData);

		VT_NODISCARD VT_INLINE const uint32_t Get() const { return m_uuid; }

		static EntityID Null();

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

	template <>
	struct formatter<Volt::EntityID> : formatter<string>
	{
		auto format(Volt::EntityID id, format_context& ctx) const
		{
			return formatter<string>::format(
			  std::format("{}", id.Get()), ctx);
		}
	};
}
