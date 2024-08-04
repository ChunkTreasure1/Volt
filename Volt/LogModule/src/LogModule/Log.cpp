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

void Log::LogMessage(LogSeverity severity, const std::string& category, const std::string& message)
{
	std::string finalString = category.empty() ? "" : "[" + category + "] ";
	finalString += message;

	switch (severity)
	{
		case LogSeverity::Trace:
			m_logger->trace(message);
			break;
		case LogSeverity::Info:
			m_logger->info(message);
			break;
		case LogSeverity::Warning:
			m_logger->warn(message);
			break;
		case LogSeverity::Error:
			m_logger->error(message);
			break;
		case LogSeverity::Critical:
			m_logger->critical(message);
			break;
	}
}
