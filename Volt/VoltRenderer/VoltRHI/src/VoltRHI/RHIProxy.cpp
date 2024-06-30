#include "rhipch.h"
#include "RHIProxy.h"

#include "VoltRHI/RHILog.h"

namespace Volt::RHI
{
	RHIProxy::~RHIProxy()
	{
		m_logger = nullptr;
	}

	RHIProxy::RHIProxy()
	{
		m_logger = CreateScope<RHILog>();
	}

	void RHIProxy::SetLogInfo(const LogInfo& logInfo)
	{
		m_logger->Initialize(logInfo);
	}
}
