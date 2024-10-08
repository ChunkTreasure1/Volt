#pragma once
#include "Nexus/Core/Core.h"

namespace Nexus
{
	inline std::string GetIp(sockaddr_in p)
	{
		char* ip = inet_ntoa(p.sin_addr);
		std::string s;
		s.resize(INET6_ADDRSTRLEN);
		memcpy_s(s.data(), s.size(), ip, INET6_ADDRSTRLEN);
		return s;
	}

	inline sockaddr_in CreateSockAddr(const std::string& in_ip, const unsigned short& in_port)
	{
		sockaddr_in sAddr;
		sAddr.sin_family = AF_INET;
		sAddr.sin_port = htons(in_port);
		inet_pton(AF_INET, in_ip.c_str(), &sAddr.sin_addr);
		return sAddr;
	}
}
