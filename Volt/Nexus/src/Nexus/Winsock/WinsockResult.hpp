#pragma once

namespace Nexus
{
	class WinsockResult
	{
		int m_iResult = 0;
	public:
		// returns true if winsock failure
		friend int operator<<(WinsockResult& in_rslt, const int& in_iResult)
		{
			in_rslt.m_iResult = in_iResult;
			return in_rslt.m_iResult;
		}
		friend bool operator==(WinsockResult& in_rslt, const int& in_iResult) { return (in_rslt.m_iResult == in_iResult); }
		bool operator!() { return m_iResult; }
		WinsockResult() {}
		WinsockResult(int in_iResult) { m_iResult = in_iResult; }
		WinsockResult(const WinsockResult& in_rslt) { m_iResult = in_rslt.m_iResult; }
		const int& Failure(void) const { return m_iResult; }
	};
}
