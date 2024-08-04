#pragma once

#include "Config.h"

#include <CoreUtilities/Assert.h>
#include <CoreUtilities/Containers/Vector.h>

#include <memory>
#include <mutex>
#include <format>
#include <filesystem>
#include <functional>

enum class LogSeverity
{
	Trace = 0,
	Info,
	Warning,
	Error,
	Critical
};

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

	LogSeverity severity;
};

class VTLOG_API Log
{
public:
	Log();
	~Log();

	template<typename... Args>
	static void LogFormatted(LogSeverity severity, const std::string& category, const std::string& format, Args&&... args)
	{
		const std::string message = std::vformat(format, std::make_format_args(args...));
		Get().LogMessage(severity, category, message);
	}

	void SetLogOutputFilepath(const std::filesystem::path& path);
	void AddCallback(const std::function<void(const LogCallbackData& callbackData)>& callback);

	VT_NODISCARD VT_INLINE static Log& Get() { return *s_instance; }

private:
	void LogMessage(LogSeverity severity, const std::string& category, const std::string& message);

	inline static Log* s_instance = nullptr;

	std::shared_ptr<spdlog::logger> m_logger;
	std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_rotatingFileSink;

	Vector<std::function<void(const LogCallbackData& callbackData)>> m_callbacks;
};

#define VT_LOGC(severity, category, format, ...) ::Log::LogFormatted(severity, category, format, __VA_ARGS__)
#define VT_LOG(severity, format, ...) ::Log::LogFormatted(severity, "", format, __VA_ARGS__)

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
