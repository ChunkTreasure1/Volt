#pragma once
#include <stdint.h>
#include <Nexus/Utility/Types.h>
#include <Wire/Serialization.h>
#include <stdint.h>

namespace Volt
{
	// Needs to match c#
	SERIALIZE_ENUM((enum class eNetEvent : uint8_t
	{
		NIL,
		Hit,
		Death,
		OnCreation,
		OnDestruction,
		Animation,
		Interact
	}), eNetEvent);

	class NetEvent
	{
	public:
		NetEvent(eNetEvent in_event, Nexus::TYPE::REP_ID in_id, std::vector<uint8_t> in_data) : m_event(in_event), m_id(in_id), m_data(in_data) {}
		NetEvent() {}
		~NetEvent() {}

		eNetEvent GetEvent() { return m_event; }
		void SetEvent(eNetEvent in_event) { m_event = in_event; }

		Nexus::TYPE::REP_ID GetId() { return m_id; }
		void SetId(Nexus::TYPE::REP_ID in_id) { m_id = in_id; }

		std::vector<uint8_t> GetData() { return m_data; }
		void SetData(std::vector<uint8_t> in_data) { m_data = in_data; }

		std::vector<uint8_t> m_data;
		eNetEvent m_event = eNetEvent::NIL;
		Nexus::TYPE::REP_ID m_id = 0;
	};
}
