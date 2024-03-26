#pragma once

namespace Volt
{
	class BinaryStreamReader;
	class BinaryStreamWriter;

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
		
		static void Serialize(BinaryStreamWriter& streamWriter, const EntityID& data);
		static void Deserialize(BinaryStreamReader& streamReader, EntityID& outData);

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
