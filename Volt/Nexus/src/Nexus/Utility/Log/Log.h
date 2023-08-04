#pragma once
#include <string>
#include <Volt/Log/Log.h>

inline static void LogTrace(const std::string& _error)
{
	VT_CORE_TRACE(_error);
}

inline static void LogInfo(const std::string& _error)
{
	VT_CORE_INFO(_error);
}

inline static void LogWarning(const std::string& _error)
{
	VT_CORE_WARN(_error);
}

inline static void LogError(const std::string& _error)
{
	VT_CORE_ERROR(_error);
}

inline static void LogCritical(const std::string& _error)
{
	VT_CORE_CRITICAL(_error);
}
