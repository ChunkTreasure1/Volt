#include "rhipch.h"
#include "RHILog.h"

namespace Volt::RHI
{
	RHILog::RHILog()
	{
		VT_ASSERT(s_instance == nullptr && "Instance should not be set!");
		s_instance = this;
	}
	
	RHILog::~RHILog()
	{
		s_instance = nullptr;
	}

	void RHILog::Initialize(const LogInfo& logInfo)
	{
		m_logInfo = logInfo;
	}

	void RHILog::LogInternal(LogSeverity severity, std::string_view message)
	{
		if (!m_logInfo.enabled || !m_logInfo.logCallback)
		{
			return;
		}

		m_logInfo.logCallback(severity, message);
	}
}
