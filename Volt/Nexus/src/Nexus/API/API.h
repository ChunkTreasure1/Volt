#pragma once
#include "Nexus/CONFIG.h"

#if (NXS_API_CONFIG == NXS_API_WINSOCK)
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Ws2tcpip.h>

#define NXS_API_UTYPE_ADDRDESC sockaddr_in
#define NXS_API_UTYPE_SOCKET SOCKET
#define NXS_API_UTYPE_SESSION WSADATA

#endif


#if (NXS_API_CONFIG == NXS_API_ASIO)
#define NXS_API_UTYPE_ADDRDESC 
#define NXS_API_UTYPE_SOCKET 
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



