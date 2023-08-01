#include "nexuspch.h"
#include "Nexus/Core/Session.h"
#if NXS_API_CONFIG == NXS_API_WINSOCK

#include <string>

namespace Nexus
{
	void Session::Start()
	{
		if (m_isValid) return;
		if (m_result << WSAStartup(MAKEWORD(2, 2), &m_session))
		{
			ErrorPrint();
		}
		m_isValid = true;
	}
	void Session::Clean()
	{
		WSACleanup();
		m_isValid = false;
	}

	bool Session::IsValid()
	{
		return m_isValid;
	}

	int Session::LastError()
	{
		return WSAGetLastError();
	}

	Result Session::GetResult()
	{
		return m_result;
	}

	const NXS_API_UTYPE_SESSION& Session::GetRaw()
	{
		return m_session;
	}

	void Session::ErrorPrint()
	{
		std::string message = "Winsock Error: ";
		message += std::to_string(LastError());
		//LogError(message);
	}
}
#endif
