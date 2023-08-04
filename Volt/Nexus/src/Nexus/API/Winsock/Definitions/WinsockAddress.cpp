#include "nexuspch.h"
#include "Nexus/Core/Address.h"
#if NXS_API_CONFIG == NXS_API_WINSOCK

namespace Nexus
{
	Address::Address(std::string _ipv4, unsigned short _port) 
		: m_address(ConstructDescription(_ipv4, _port))
	{
	}
	Address::Address(Address& _desc) : m_address(_desc.m_address)
	{

	}

	Address::Address(const Address& _desc) : m_address(_desc.m_address)
	{
	}

	Address::Address(const NXS_API_UTYPE_ADDRDESC& _desc) : m_address(_desc)
	{
	}

	Address::~Address()
	{
	}

	std::string Address::String()
	{
		if(m_flag != eAddressFlag::NIL)
		return std::string();
	}

	eAddressFlag Address::Flag()
	{
		return m_flag;
	}

	unsigned short Address::Port()
	{
		return 0;
	}

	std::string Address::IPV4()
	{
		char* ip = inet_ntoa(m_address.sin_addr);
		std::string s;
		s.resize(INET6_ADDRSTRLEN);
		memcpy_s(s.data(), s.size(), ip, INET6_ADDRSTRLEN);
		return s;
	}

	bool operator==(Address _one, Address _two)
	{
		return false;
	}

	NXS_API_UTYPE_ADDRDESC Address::ConstructDescription(std::string _ipv4, unsigned short _port)
	{
		NXS_API_UTYPE_ADDRDESC sAddr;
		sAddr.sin_family = AF_INET;
		sAddr.sin_port = htons(_port);
		inet_pton(AF_INET, _ipv4.c_str(), &sAddr.sin_addr);
		return sAddr;
	}

	Address Address::ConstructAddress(std::string _ipv4, unsigned short _port)
	{
		return Address(ConstructDescription(_ipv4, _port));
	}

	Address Address::ConstructNilAddress()
	{
		return Address();
	}
}
#endif
