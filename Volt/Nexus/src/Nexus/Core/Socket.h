#pragma once
#include "Nexus/API/API.h"
#include "Nexus/Core/Result.h"
#include <string>

namespace Nexus
{
	class Socket
	{
	public:
		Socket();
		~Socket();

		bool Init();
		void Bind(unsigned short& in_port);
		void Close();

		bool IsValid() { return (m_socket != 0); }
		void SocketError();

		bool Send(Address& in_address, const char* buffer, int len, int flags = 0);
		bool Recieve(Address& out_addr, char* out_buffer, int& out_size, int len, int flags = 0);

	private:
		Result m_result;
		NXS_API_UTYPE_SOCKET m_socket;
	};
}
