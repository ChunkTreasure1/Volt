#include "vtpch.h"
#include "Volt/Log/Log.h"
#include "Volt/Project/ProjectManager.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Volt
{
	void Log::Initialize()
	{
		myCallbackSink = CreateRef<CallbackSink<std::mutex>>();
		
		spdlog::set_pattern("%^[%T] %n: %v%$");

		myClientLogger = spdlog::stdout_color_mt("APP");
		myClientLogger->sinks().emplace_back(myCallbackSink);
		myClientLogger->set_level(spdlog::level::trace);

		myCoreLogger = spdlog::stdout_color_mt("VOLT");
		myCoreLogger->sinks().emplace_back(myCallbackSink);
		myCoreLogger->set_level(spdlog::level::trace);
	}

	void Log::InitializeFileSinks()
	{
		auto max_size = 1048576 * 5;
		auto max_files = 3;

		const std::filesystem::path outputPath = ProjectManager::GetDirectory() / "Log/Log.txt";

		if (!std::filesystem::exists(outputPath))
		{
			std::filesystem::create_directories(outputPath.parent_path());

			std::ofstream out{ outputPath };
			out.close();
		}

		myRotatingFileLogger = CreateRef<spdlog::sinks::rotating_file_sink_mt>(outputPath.string(), max_size, max_files, false);

		myClientLogger->sinks().emplace_back(myRotatingFileLogger);
		myCoreLogger->sinks().emplace_back(myRotatingFileLogger);
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
