#pragma once
#define WIN32_LEAN_AND_MEAN

// Winsock
#include <WinSock2.h>
#include <ws2tcpip.h>

// Nexus
#include "Nexus/Common.h"
#include "Nexus/Winsock/WinsockResult.hpp"
#include "Nexus/Winsock/WSASession.hpp"
#include "Nexus/Core/Defines.h"

inline bool operator==(const sockaddr_in& addr1, const sockaddr_in& addr2)
{
	return (
		addr1.sin_addr.s_addr == addr2.sin_addr.s_addr &&
		addr1.sin_port == addr2.sin_port &&
		addr1.sin_family == addr2.sin_family
		);
}
