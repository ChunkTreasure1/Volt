#pragma once
#include "Nexus/Core/Core.h"

namespace Nexus
{
	class UDPSocket
	{
	public:
		UDPSocket();
		~UDPSocket();

		bool Init();
		void Bind(unsigned short in_port);
		void Close();

		bool IsValid() { return (m_sWin != INVALID_SOCKET); }

		void SendTo(const std::string& in_address, unsigned short in_port, const char* buffer, int len, int flags = 0);
		void SendTo(sockaddr_in& in_address, const char* buffer, int len, int flags = 0);
		sockaddr_in RecvFrom(char* out_buffer, int& out_size, int len, int flags = 0);

		void SocketError(bool in_checkResult = false);
		
	private:
		SOCKET m_sWin = INVALID_SOCKET;
		WinsockResult m_result;
	};
}
