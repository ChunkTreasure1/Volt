#pragma once
#include "Nexus/Utility/tsdeque.h"
#include "Nexus/Packet/AddressedPacket.h"

namespace Nexus
{
	class PacketQueue
	{
	public:
		PacketQueue() = default;
		~PacketQueue() = default;

		void Add(const AddressedPacket& _addrPacket);
		AddressedPacket Get();

		//tsdeque<AddressedPacket> Queue();

	private:
		tsdeque<AddressedPacket> m_queue;
	};
}
