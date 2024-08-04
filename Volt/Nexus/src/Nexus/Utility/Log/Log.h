#pragma once
#include <string>
#include <LogModule/Log.h>

inline static void LogTrace(const std::string& in_error)
{
	VT_LOG(LogSeverity::Trace, in_error);
}

inline static void LogInfo(const std::string& in_error)
{
	VT_LOG(LogSeverity::Info, in_error);
}

inline static void LogWarning(const std::string& in_error)
{
	VT_LOG(LogSeverity::Warning, in_error);
}

inline static void LogError(const std::string& in_error)
{
	VT_LOG(LogSeverity::Error, in_error);
}

inline static void LogCritical(const std::string& in_error)
{
	VT_LOG(LogSeverity::Critical, in_error);
}
