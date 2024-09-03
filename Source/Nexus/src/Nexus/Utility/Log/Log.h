#pragma once
#include <string>
#include <LogModule/Log.h>

inline static void LogTrace(const std::string& in_error)
{
	VT_LOG(Trace, in_error);
}

inline static void LogInfo(const std::string& in_error)
{
	VT_LOG(Info, in_error);
}

inline static void LogWarning(const std::string& in_error)
{
	VT_LOG(Warning, in_error);
}

inline static void LogError(const std::string& in_error)
{
	VT_LOG(Error, in_error);
}

inline static void LogCritical(const std::string& in_error)
{
	VT_LOG(Critical, in_error);
}
