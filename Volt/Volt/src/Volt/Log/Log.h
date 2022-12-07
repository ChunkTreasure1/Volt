#pragma once

#include "CallbackSink.h"

#include "Volt/Core/Base.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Volt
{
	class Log
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void SetLogLevel(spdlog::level::level_enum level);
		static void AddCallback(std::function<void(const LogCallbackData&)> callback);

		inline static Ref<spdlog::logger> GetClientLogger() { return myClientLogger; }
		inline static Ref<spdlog::logger> GetCoreLogger() { return myCoreLogger; }

	private:
		inline static Ref<spdlog::logger> myClientLogger;
		inline static Ref<spdlog::logger> myCoreLogger;

		inline static Ref<CallbackSink<std::mutex>> myCallbackSink;
	};

//Client logging macros
#define VT_TRACE(...)			::Volt::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VT_INFO(...)			::Volt::Log::GetClientLogger()->info(__VA_ARGS__)
#define VT_WARN(...)			::Volt::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VT_ERROR(...)			::Volt::Log::GetClientLogger()->error(__VA_ARGS__)
#define VT_CRITICAL(...)		::Volt::Log::GetClientLogger()->critical(__VA_ARGS__)

//Core logging macros
#define VT_CORE_TRACE(...)		::Volt::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VT_CORE_INFO(...)		::Volt::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VT_CORE_WARN(...)		::Volt::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VT_CORE_ERROR(...)		::Volt::Log::GetCoreLogger()->error(__VA_ARGS__)
#define VT_CORE_CRITICAL(...)	::Volt::Log::GetCoreLogger()->critical(__VA_ARGS__)
}