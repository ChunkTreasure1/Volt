#pragma once
#include "Nexus/API/API.h"

namespace Nexus
{
	class Result
	{
	public:
		// returns true if winsock failure
		friend int operator<<(Result& result, const int& in_result)
		{
			result.m_result = in_result;
			return result.m_result;
		}
		friend bool operator==(Result& result, const int& in_result) { return (result.m_result == in_result); }
		bool operator!() { return m_result; }
		Result() {}
		Result(int in_result) { m_result = in_result; }
		Result(const Result& in_result) { m_result = in_result.m_result; }
		const int& Failure(void) const { return m_result; }
	private:
		int m_result = 0;
	};
}
