#include "nexuspch.h"
#include "Guarantor.h"
#include "Nexus/Utility/Random/Random.h"

namespace Nexus
{
	Guarantor::Guarantor(tsdeque<std::pair<Address, Packet>>& _q, std::atomic<bool>& running)
		: ex_outQue(_q), ex_isRunning(running)
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

	TYPE::NOTIFY_ID Guarantor::Add(const Address& _address, const Packet& _packet)
	{
		// ts
		if (_packet.notifyId == 0) return 0;
		if (_packet.id == ePacketID::NOTIFY_CONFIRMED) return _packet.notifyId;

		TYPE::NOTIFY_ID newId = Random<TYPE::NOTIFY_ID>();
		while (!m_map.contains(newId)) newId = Random<TYPE::NOTIFY_ID>();
		GuaranteeData data;
		data.target = _address;
		data.packet = _packet;
		data.packet.notifyId = newId;
		data.time = 0;
		data.tries = 0;
		std::pair<TYPE::NOTIFY_ID, GuaranteeData> entry;
		m_map.insert(entry);
		return newId;
	}

	bool Guarantor::Handle(const Packet& _packet, const Nexus::Address& _address)
	{
		// ts
		TYPE::NOTIFY_ID notId = _packet.notifyId;

		if (notId == 0) return true;
		if (_packet.id == ePacketID::NOTIFY_CONFIRMED)
		{
			if (!m_map.contains(notId)) return false;
			m_map.erase(notId);
			return false;
		}

		Nexus::Packet confirmationPacket;
		confirmationPacket.notifyId = notId;
		confirmationPacket.id = ePacketID::NOTIFY_CONFIRMED;
		std::pair<Nexus::Address, Nexus::Packet> entry{ _address, confirmationPacket };
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
