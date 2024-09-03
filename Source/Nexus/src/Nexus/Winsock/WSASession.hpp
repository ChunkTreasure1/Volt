#pragma once
#include "Nexus/Core/Core.h"

namespace Nexus::WSA
{
	class Session
	{
	public:
		inline static void Start()
		{
			if (m_isValid) return;
			if (m_rsltWinsock << WSAStartup(MAKEWORD(2, 2), &m_wsaData))
			{
				ErrorPrint();
			}
			m_isValid = true;
		}
		inline static void Clean() { WSACleanup(); m_isValid = false; }

		inline static bool IsValid() { return m_isValid; }
		inline static int LastError() { return WSAGetLastError(); }

		inline static WinsockResult GetRslt() { return m_rsltWinsock; }
		inline static const WSADATA& GetWSA() { return m_wsaData; }

		inline static void ErrorPrint()
		{
			std::string message = "Winsock Error: ";
			message += std::to_string(LastError());
			LogError(message);
		}


	private:
		inline static WSADATA m_wsaData;
		inline static WinsockResult m_rsltWinsock;
		inline static bool m_isValid = false;
	};
}
