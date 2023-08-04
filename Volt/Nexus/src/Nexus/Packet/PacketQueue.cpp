#include "nexuspch.h"
#include "PacketQueue.h"

namespace Nexus
{
	void PacketQueue::Add(const AddressedPacket& _addrPacket)
	{
		m_queue.push_back(_addrPacket);
	}

	AddressedPacket PacketQueue::Get()
	{
		return m_queue.pop_front();
	}
}
