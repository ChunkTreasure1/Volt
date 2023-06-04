#include "vtpch.h"
#include "Volt/Log/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Volt
{
	void Log::Initialize()
	{
		auto max_size = 1048576 * 5;
		auto max_files = 3;

		const std::filesystem::path outputPath = "Log/Log.txt";

		if (!std::filesystem::exists(outputPath))
		{
			std::filesystem::create_directories(outputPath.parent_path());
		
			std::ofstream out{ outputPath };
			out.close();
		}

		myCallbackSink = CreateRef<CallbackSink<std::mutex>>();
		myRotatingFileLogger = CreateRef<spdlog::sinks::rotating_file_sink_mt>(outputPath.string(), max_size, max_files, false);
		
		spdlog::set_pattern("%^[%T] %n: %v%$");

		myClientLogger = spdlog::stdout_color_mt("APP");
		myClientLogger->sinks().emplace_back(myCallbackSink);
		myClientLogger->sinks().emplace_back(myRotatingFileLogger);
		myClientLogger->set_level(spdlog::level::trace);

		myCoreLogger = spdlog::stdout_color_mt("VOLT");
		myCoreLogger->sinks().emplace_back(myCallbackSink);
		myCoreLogger->sinks().emplace_back(myRotatingFileLogger);
		myCoreLogger->set_level(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		myCallbackSink->ClearCallbacks();
		myCallbackSink = nullptr;
		myRotatingFileLogger = nullptr;
	}

	void Log::SetLogLevel(spdlog::level::level_enum level)
	{
		myCoreLogger->set_level(level);
		myClientLogger->set_level(level);
	}

	void Log::AddCallback(std::function<void(const LogCallbackData&)> callback)
	{
		myCallbackSink->AddCallback(callback);
	}

	void Log::ClearCallbacks()
	{
		myCallbackSink->ClearCallbacks();
	}
}
