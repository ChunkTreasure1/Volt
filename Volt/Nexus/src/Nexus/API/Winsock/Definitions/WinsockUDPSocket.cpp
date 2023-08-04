#include "nexuspch.h"
#include "Nexus/Core/Socket.h"
#if NXS_API_CONFIG == NXS_API_WINSOCK

#include "Nexus/Core/Session.h"
#include "Nexus/Core/Address.h"

namespace Nexus
{
	Socket::Socket()
	{
	}

	Socket::~Socket()
	{
		closesocket(m_socket);
	}

	bool Socket::Init()
	{
		if (!m_socket)
		{
			SocketError();
			return false;
		}
		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_socket == INVALID_SOCKET)
		{
			Session::ErrorPrint();
			return false;
		}
		return true;
	}

	bool Socket::Send(Address& _address, const char* _buffer, int _len, int _flags)
	{
		int ret = sendto(m_socket, _buffer, _len, _flags, reinterpret_cast<SOCKADDR*>(_address.GetDescriptionPtr()), sizeof(_address.GetDescription()));

		if (ret < 0)
		{
			//SocketError();

			return false;
		}
		return true;
	}

	bool Socket::Recieve(Address& out_addr, char* out_buffer, int& out_size, int _len, int _flags)
	{
		int addrLenght = _len;
		int ret = recvfrom(m_socket, out_buffer, _len, _flags, reinterpret_cast<SOCKADDR*>(out_addr.GetDescriptionPtr()), &addrLenght);
		out_size = ret;

		if (Nexus::Session::LastError() == WSAEWOULDBLOCK)
		{
			//SocketError();
		}
		else if (ret < 0)
		{
			SocketError();
			return false;
		}


		// make the buffer zero terminated
		out_buffer[ret] = 0;
		return true;
	}

	void Socket::Bind(unsigned short& _port)
	{
		sockaddr_in add;
		add.sin_family = AF_INET;
		add.sin_addr.s_addr = htonl(INADDR_ANY);
		add.sin_port = htons(_port);

		if (m_result << bind(m_socket, reinterpret_cast<SOCKADDR*>(&add), sizeof(add)))
		{
			SocketError();
			return;
		}

		// port
		struct sockaddr_in sin;
		int addrlen = sizeof(sin);
		if (getsockname(m_socket, (struct sockaddr*)&sin, &addrlen)
			|| sin.sin_family != AF_INET
			|| addrlen != sizeof(sin))
		{
			SocketError();
			_port = 0;
			Close();
			return;
		}
		_port = ntohs(sin.sin_port);

#if NXS_PROTOCOL == NXS_PROTOCOL_UDP_NONBLOCKING
		unsigned long nonblocking = 1;
		if (ioctlsocket(m_socket, FIONBIO, &nonblocking))
		{
			SocketError(false);
			in_port = 0;
			Close();
			return;
		}
#endif
	}

	void Socket::Close()
	{
		closesocket(m_socket);
		m_result = INVALID_SOCKET;
	}

	void Socket::SocketError()
	{
		if (m_result.Failure())
		{
			//LogError("Socket error: " + std::to_string(m_result.Failure()));
		}
		//LogError("Session error: " + std::to_string(Nexus::Session::LastError()));
		return;
	}
}


#endif
