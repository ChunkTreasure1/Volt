#pragma once
#include "Nexus/API/API.h"
#include <string>

namespace Nexus
{
	enum class eAddressFlag : uint8_t
	{
		NIL,
		VALID,
	};

	using Port = NXS_API_UTYPE_PORT;
	using IPv4 = NXS_API_UTYPE_IPV4;

	class Address
	{
	public:
		Address() = default;
		Address(std::string _ipv4, unsigned short _port);
		Address(Address& _desc);
		Address(const Address& _desc);
		Address(const NXS_API_UTYPE_ADDRDESC& _desc);
		~Address();

		static NXS_API_UTYPE_ADDRDESC ConstructDescription(std::string _ipv4, unsigned short _port);
		static Address ConstructAddress(std::string _ipv4, unsigned short _port);
		static Address ConstructNilAddress();

		const NXS_API_UTYPE_ADDRDESC& GetDescription() const { return m_address; };
		NXS_API_UTYPE_ADDRDESC* GetDescriptionPtr() { return &m_address; };

		friend bool operator==(Address _one, Address _two);

		unsigned short Port();
		std::string IPV4();
		std::string String();
		eAddressFlag Flag();

	private:
		eAddressFlag m_flag = eAddressFlag::NIL;
		NXS_API_UTYPE_ADDRDESC m_address;
	};
}
