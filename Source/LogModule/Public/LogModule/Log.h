#pragma once

#include "LogModule/Config.h"
#include "LogModule/LogCommon.h"
#include "LogModule/LogCategory.h"
#include "LogModule/DefaultLoggingCategories.h"

#include <CoreUtilities/VoltAssert.h>
#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/UUID.h>

#include <memory>
#include <mutex>
#include <format>
#include <filesystem>
#include <functional>

namespace spdlog
{
	class logger;

	namespace sinks
	{
		template<typename Mutex>
		class rotating_file_sink;

		using rotating_file_sink_mt = rotating_file_sink<std::mutex>;
	}
}

struct LogCallbackData
{
	std::string category;
	std::string message;

	LogVerbosity severity;
};

typedef UUID32 LogCallbackHandle;

class VTLOG_API Log
{
public:
	Log();
	~Log();

	template<typename LogCategory, typename... Args>
	static void LogFormatted(LogVerbosity severity, const LogCategory& category, const std::string& format, Args&&... args)
	{
		const std::string message = std::vformat(format, std::make_format_args(args...));
		Get().LogMessage(severity, std::string(category.GetName()), message);
	}

	void SetLogOutputFilepath(const std::filesystem::path& path);
	LogCallbackHandle RegisterCallback(const std::function<void(const LogCallbackData& callbackData)>& callback);
	void UnregisterCallback(LogCallbackHandle handle);

	VT_NODISCARD VT_INLINE static Log& Get() { return *s_instance; }

private:
	void LogMessage(LogVerbosity severity, const std::string& category, const std::string& message);

	inline static Log* s_instance = nullptr;

	std::shared_ptr<spdlog::logger> m_logger;
	std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_rotatingFileSink;

	struct CallbackData
	{
		std::function<void(const LogCallbackData& callbackData)> callbackFunc;
		LogCallbackHandle handle;
	};

	std::mutex m_callbackMutex;
	Vector<CallbackData> m_callbacks;
};

#define VT_LOGC(verbosity, category, format, ...) ::Log::LogFormatted(LogVerbosity::verbosity, category, format, __VA_ARGS__)
#define VT_LOG(verbosity, format, ...) ::Log::LogFormatted(LogVerbosity::verbosity, LogTemp, format, __VA_ARGS__)

// Special formatters
namespace std
{
	template <>
	struct formatter<filesystem::path> : formatter<string>
	{
		auto format(filesystem::path p, format_context& ctx) const
		{
			return formatter<string>::format(
			  std::format("{}", p.string()), ctx);
		}
	};
}
