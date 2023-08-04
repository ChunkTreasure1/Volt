#pragma once
#include "Nexus/API/API.h"

namespace Nexus
{
	class Result
	{
	public:
		// returns true if winsock failure
		friend int operator<<(Result& result, const int& _result)
		{
			result.m_result = _result;
			return result.m_result;
		}
		friend bool operator==(Result& result, const int& _result) { return (result.m_result == _result); }
		bool operator!() { return m_result; }
		Result() {}
		Result(int _result) { m_result = _result; }
		Result(const Result& _result) { m_result = _result.m_result; }
		const int& Failure(void) const { return m_result; }
	private:
		int m_result = 0;
	};
}
