#pragma once
#include "Nexus/API/CONFIG.h"

#if (NXS_API_CONFIG == NXS_API_WINSOCK)
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string>

#define NXS_API_UTYPE_ADDRDESC sockaddr_in
#define NXS_API_UTYPE_SOCKET SOCKET
#define NXS_API_UTYPE_NILSOCKET INVALID_SOCKET
#define NXS_API_UTYPE_SESSION WSADATA

#define NXS_API_UTYPE_PORT unsigned short
#define NXS_API_UTYPE_IPV4 std::string;

#endif


#if (NXS_API_CONFIG == NXS_API_ASIO)
#define NXS_API_UTYPE_ADDRDESC 
#define NXS_API_UTYPE_SOCKET 
#define NXS_API_UTYPE_NILSOCKET
#define NXS_API_UTYPE_SESSION

#endif

namespace Nexus
{
	class Address;
	class Socket;
	class Result;
	class Session;
}

// function decl



