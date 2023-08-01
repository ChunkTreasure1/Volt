#pragma once
#include "Nexus/API/API.h"
#include <string>

namespace Nexus
{
	class Address
	{
	public:
		Address() = default;
		Address(std::string ipv4, unsigned short port);
		Address(Address& desc);
		Address(const Address& desc);
		Address(const NXS_API_UTYPE_ADDRDESC& desc);
		~Address();

		static NXS_API_UTYPE_ADDRDESC ConstructDescription(std::string ipv4, unsigned short port);
		static Address ConstructAddress(std::string ipv4, unsigned short port);

		const NXS_API_UTYPE_ADDRDESC& GetDescription() const { return m_address; };
		NXS_API_UTYPE_ADDRDESC* GetDescriptionPtr() { return &m_address; };

		friend bool operator==(Address one, Address two);

		unsigned short Port();
		std::string IPV4();
		std::string String();

	private:
		NXS_API_UTYPE_ADDRDESC m_address;
	};
}
