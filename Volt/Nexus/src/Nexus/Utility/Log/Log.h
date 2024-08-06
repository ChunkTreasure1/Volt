#pragma once
#include <string>
#include <LogModule/Log.h>

inline static void LogTrace(const std::string& in_error)
{
	VT_LOG(LogVerbosity::Trace, in_error);
}

inline static void LogInfo(const std::string& in_error)
{
	VT_LOG(LogVerbosity::Info, in_error);
}

inline static void LogWarning(const std::string& in_error)
{
	VT_LOG(LogVerbosity::Warning, in_error);
}

inline static void LogError(const std::string& in_error)
{
	VT_LOG(LogVerbosity::Error, in_error);
}

inline static void LogCritical(const std::string& in_error)
{
	VT_LOG(LogVerbosity::Critical, in_error);
}
