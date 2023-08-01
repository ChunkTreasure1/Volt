#include "nexuspch.h"
#include "Guarantor.h"

namespace Nexus
{
	Guarantor::Guarantor(tsdeque<std::pair<Address, Packet>>& in_q, std::atomic<bool>& running)
		: ex_outQue(in_q), ex_isRunning(running)
	{
	}

	Guarantor::~Guarantor()
	{
	}

	void Guarantor::Update()
	{
		while (ex_isRunning)
		{
			// resending
			// clearing handled
			// handle time
		}
		
		Clear();
	}

	void Guarantor::Clear()
	{
		//ts
		m_handled.clear();
		m_map.clear();
		m_droppedPacketCount = 0;
	}

	TYPE::NOTIFY_ID Guarantor::Add(const Address& in_address, const Packet& in_packet)
	{
		// ts
		if (in_packet.notifyId == 0) return 0;
		if (in_packet.id == ePacketID::NOTIFY_CONFIRMED) return in_packet.notifyId;

		TYPE::NOTIFY_ID newId = TYPE::RandNotifyID();
		while (!m_map.contains(newId)) newId = TYPE::RandNotifyID();
		GuaranteeData data;
		data.target = in_address;
		data.packet = in_packet;
		data.packet.notifyId = newId;
		data.time = 0;
		data.tries = 0;
		std::pair<TYPE::NOTIFY_ID, GuaranteeData> entry;
		m_map.insert(entry);
		return newId;
	}

	bool Guarantor::Handle(const Packet& in_packet, const Nexus::Address& in_address)
	{
		// ts
		TYPE::NOTIFY_ID notId = in_packet.notifyId;

		if (notId == 0) return true;
		if (in_packet.id == ePacketID::NOTIFY_CONFIRMED)
		{
			if (!m_map.contains(notId)) return false;
			m_map.erase(notId);
			return false;
		}

		Nexus::Packet confirmationPacket;
		confirmationPacket.notifyId = notId;
		confirmationPacket.id = ePacketID::NOTIFY_CONFIRMED;
		std::pair<Nexus::Address, Nexus::Packet> entry{ in_address, confirmationPacket };
		ex_outQue.push_back(entry);

		if (m_handled.contains(notId))
		{
			// count time
			m_droppedPacketCount++;
			return false;
		}

		m_handled[notId] = 0;
		return true;
	}
}
