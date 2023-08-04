#pragma once
#include "Nexus/Core/Address.h"
#include "Nexus/Packet/Packet.h"

namespace Nexus
{
	class AddressedPacket
	{
	public:
		AddressedPacket();
		AddressedPacket(const Nexus::Address& _address);
		AddressedPacket(const Nexus::Packet& _packet);
		AddressedPacket(const Nexus::Address& _address, const Nexus::Packet& _packet);
		AddressedPacket(const Nexus::Packet& _packet, const Nexus::Address& _address);
		~AddressedPacket();

		const Nexus::Address& Address();
		const Nexus::Packet& Packet();
		const std::pair<Nexus::Address, Nexus::Packet>& Pair();

	private:
		std::pair<Nexus::Address, Nexus::Packet> m_pair;
	};
}
