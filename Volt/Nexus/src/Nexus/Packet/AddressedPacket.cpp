#include "nexuspch.h"
#include "AddressedPacket.h"

namespace Nexus
{
	Nexus::AddressedPacket::AddressedPacket()
	{
	}

	Nexus::AddressedPacket::AddressedPacket(const Nexus::Address& _address)
	{
		AddressedPacket(_address, Nexus::Packet());
	}

	Nexus::AddressedPacket::AddressedPacket(const Nexus::Packet& _packet)
	{
		AddressedPacket(Nexus::Address::ConstructNilAddress(), _packet);
	}

	Nexus::AddressedPacket::AddressedPacket(const Nexus::Address& _address, const Nexus::Packet& _packet)
	{
		m_pair = { _address, _packet };
	}

	Nexus::AddressedPacket::AddressedPacket(const Nexus::Packet& _packet, const Nexus::Address& _address)
	{
		AddressedPacket(_address, _packet);
	}

	Nexus::AddressedPacket::~AddressedPacket()
	{
	}

	const Nexus::Address& Nexus::AddressedPacket::Address()
	{
		return m_pair.first;
	}

	const Nexus::Packet& Nexus::AddressedPacket::Packet()
	{
		return m_pair.second;
	}

	const std::pair<Nexus::Address, Nexus::Packet>& Nexus::AddressedPacket::Pair()
	{
		return m_pair;
	}
}
