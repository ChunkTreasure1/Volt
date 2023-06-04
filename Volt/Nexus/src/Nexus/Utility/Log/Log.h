#pragma once
#include <string>
#include <Volt/Log/Log.h>

inline static void LogTrace(const std::string& in_error)
{
	VT_CORE_TRACE(in_error);
}

inline static void LogInfo(const std::string& in_error)
{
	VT_CORE_INFO(in_error);
}

inline static void LogWarning(const std::string& in_error)
{
	VT_CORE_WARN(in_error);
}

inline static void LogError(const std::string& in_error)
{
	VT_CORE_ERROR(in_error);
}

inline static void LogCritical(const std::string& in_error)
{
	VT_CORE_CRITICAL(in_error);
}
