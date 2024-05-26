#pragma once

#include "VoltRHI/Core/RHICommon.h"

#include <format>

namespace Volt::RHI
{
	class VTRHI_API RHILog
	{
	public:
		RHILog();
		~RHILog();
		
		void Initialize(const LogInfo& logInfo);

		template<typename... Args>
		VT_INLINE static void Log(LogSeverity severity, std::string_view message, Args&&... args)
		{
			GetInstance().LogInternal(severity, std::vformat(message, std::make_format_args(args...)));
		}

		template<typename... Args>
		VT_INLINE static void LogTagged(LogSeverity severity, std::string_view tag, std::string_view message, Args&&... args)
		{
			GetInstance().LogInternal(severity, std::format("{0}: {1}", tag, std::vformat(message, std::make_format_args(args...))));
		}

		VT_INLINE static void LogUnformatted(LogSeverity severity, std::string_view message)
		{
			GetInstance().LogInternal(severity, message);
		}

	private:
		void LogInternal(LogSeverity severity, std::string_view message);
		VT_INLINE static RHILog& GetInstance() { return *s_instance; }

		inline static RHILog* s_instance = nullptr;

		LogInfo m_logInfo;
	};
}
