#include "Log.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

Log::Log()
{
	VT_ENSURE(s_instance == nullptr);
	s_instance = this;

	spdlog::set_pattern("%^[%T] %n: %v%$");

	m_logger = spdlog::stdout_color_mt("VOLT");
	m_logger->set_level(spdlog::level::trace);
}

Log::~Log()
{
	m_rotatingFileSink = nullptr;
	m_logger = nullptr;

	s_instance = nullptr;
}

void Log::SetLogOutputFilepath(const std::filesystem::path& path)
{
	auto max_size = 1048576 * 5;
	auto max_files = 3;

	if (!std::filesystem::exists(path.parent_path()))
	{
		std::filesystem::create_directories(path.parent_path());
	}

	m_rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path.string(), max_size, max_files, false);
	m_logger->sinks().emplace_back(m_rotatingFileSink);
}

LogCallbackHandle Log::RegisterCallback(const std::function<void(const LogCallbackData& callbackData)>& callback)
{
	LogCallbackHandle handle = {};
	m_callbacks.emplace_back(callback, handle);
	return handle;
}

void Log::UnregisterCallback(LogCallbackHandle handle)
{
	auto it = std::find_if(m_callbacks.begin(), m_callbacks.end(), [handle](const auto& data)
	{
		return data.handle == handle;
	});

	std::scoped_lock lock{ m_callbackMutex };
	if (it != m_callbacks.end())
	{
		m_callbacks.erase(it);
	}
}

void Log::LogMessage(LogVerbosity severity, const std::string& category, const std::string& message)
{
	std::string finalString = category.empty() ? "" : "[" + category + "]: ";
	finalString += message;

	switch (severity)
	{
		case LogVerbosity::Trace:
			m_logger->trace(finalString);
			break;
		case LogVerbosity::Info:
			m_logger->info(finalString);
			break;
		case LogVerbosity::Warning:
			m_logger->warn(finalString);
			break;
		case LogVerbosity::Error:
			m_logger->error(finalString);
			break;
		case LogVerbosity::Critical:
			m_logger->critical(finalString);
			break;
	}

	LogCallbackData callbackData{};
	callbackData.category = category;
	callbackData.message = finalString;
	callbackData.severity = severity;

	std::scoped_lock lock{ m_callbackMutex };
	for (const auto& data : m_callbacks)
	{
		data.callbackFunc(callbackData);
	}
}
