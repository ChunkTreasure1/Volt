#include "nexuspch.h"
#include "Nexus/Core/Address.h"
#if NXS_API_CONFIG == NXS_API_WINSOCK

namespace Nexus
{
	Address::Address(std::string ipv4, unsigned short port) 
		: m_address(ConstructDescription(ipv4, port))
	{
	}
	Address::Address(Address& desc) : m_address(desc.m_address)
	{

	}

	Address::Address(const Address& desc) : m_address(desc.m_address)
	{
	}

	Address::Address(const NXS_API_UTYPE_ADDRDESC& desc) : m_address(desc)
	{
	}

	Address::~Address()
	{
	}

	std::string Address::String()
	{

		return std::string();
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

	bool operator==(Address one, Address two)
	{
		return false;
	}

	NXS_API_UTYPE_ADDRDESC Address::ConstructDescription(std::string ipv4, unsigned short port)
	{
		NXS_API_UTYPE_ADDRDESC sAddr;
		sAddr.sin_family = AF_INET;
		sAddr.sin_port = htons(port);
		inet_pton(AF_INET, ipv4.c_str(), &sAddr.sin_addr);
		return sAddr;
	}

	Address Address::ConstructAddress(std::string ipv4, unsigned short port)
	{
		return Address(ConstructDescription(ipv4, port));
	}
}
#endif
