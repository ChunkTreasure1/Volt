#pragma once
#include "VoltRHI/Extensions/Extension.h"

namespace Volt
{
	enum class LogSeverity
	{
		Trace = 1 << 0,
		Log = 1 << 1,
		Warning = 1 << 2,
		Error = 1 << 3,
	};

	class DebugLogExtension : public Extension
	{
	public:
		DebugLogExtension() { };
		~DebugLogExtension() = default;


		void SetTrace(bool flag)	{ SetBitValue(flag, 0); }
		void SetLog(bool flag)		{ SetBitValue(flag, 1); }
		void SetWarning(bool flag)	{ SetBitValue(flag, 2); }
		void SetError(bool flag)	{ SetBitValue(flag, 3); }

		void AssignLogCallback(std::function<void(std::string_view, LogSeverity)>&& callback) { m_logCallback = callback; }

		void Log(const LogSeverity severity, const std::string& message);

		REGISTER_EXTENSION(DebugLogExtension);
		
	private:
		void SetBitValue(bool flag, int8_t bit)
		{
			auto filter = static_cast<int8_t>(m_callbackFilter);
			flag ? filter |= (1 << bit) : filter &= ~(1 << bit);
			m_callbackFilter = static_cast<LogSeverity>(filter);
		}

		std::function<void(std::string_view, LogSeverity)> m_logCallback;
		
		LogSeverity m_callbackFilter;
	};
}
