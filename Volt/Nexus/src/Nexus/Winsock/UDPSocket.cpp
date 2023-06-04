#include "nexuspch.h"
#include "UDPSocket.h"
#include "Nexus/Winsock/WSASession.hpp"

namespace Nexus
{
	UDPSocket::UDPSocket()
	{

	}

	UDPSocket::~UDPSocket()
	{
		closesocket(m_sWin);
	}

	bool UDPSocket::Init()
	{
		if (!m_sWin)
		{
			SocketError(true);
			return false;
		}
		m_sWin = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_sWin == INVALID_SOCKET)
		{
			WSA::Session::ErrorPrint();
			return false;
		}
		return true;
	}

	void UDPSocket::SendTo(const std::string& in_address, unsigned short in_port, const char* buffer, int len, int flags)
	{
		sockaddr_in add;
		add.sin_family = AF_INET;
		add.sin_port = htons(in_port);
		inet_pton(AF_INET, in_address.c_str(), &add.sin_addr);

		int ret = sendto(m_sWin, buffer, len, flags, reinterpret_cast<SOCKADDR*>(&add), sizeof(add));

		if (ret < 0)
		{
			SocketError();
			return;
		}
	}

	void UDPSocket::SendTo(sockaddr_in& in_address, const char* buffer, int len, int flags)
	{
		int ret = sendto(m_sWin, buffer, len, flags, reinterpret_cast<SOCKADDR*>(&in_address), sizeof(in_address));

		if (ret < 0)
		{
			SocketError();
			return;
		}
	}

	sockaddr_in UDPSocket::RecvFrom(char* out_buffer, int& out_size, int len, int flags)
	{
		sockaddr_in from;
		int addrLenght = len;
		int ret = recvfrom(m_sWin, out_buffer, len, flags, reinterpret_cast<SOCKADDR*>(&from), &addrLenght);
		out_size = ret;

		if (ret < 0)
		{
			SocketError();
			return from;
		}

		// make the buffer zero terminated
		out_buffer[ret] = 0;
		return from;
	}

	void UDPSocket::Bind(unsigned short in_port)
	{
		sockaddr_in add;
		add.sin_family = AF_INET;
		add.sin_addr.s_addr = htonl(INADDR_ANY);
		add.sin_port = htons(in_port);

		if (m_result << bind(m_sWin, reinterpret_cast<SOCKADDR*>(&add), sizeof(add)))
		{
			SocketError(true);
			return;
		}
	}

	void UDPSocket::Close()
	{
		closesocket(m_sWin);
		m_result = INVALID_SOCKET;
	}

	void UDPSocket::SocketError(bool in_checkResult)
	{
		if (in_checkResult)
		{
			if (m_result.Failure())
			{
				LogError("Socket error: " + std::to_string(m_result.Failure()));
				return;
			}
		}
		else
		{
			//LogError("Socket error");
		}
	}
}
