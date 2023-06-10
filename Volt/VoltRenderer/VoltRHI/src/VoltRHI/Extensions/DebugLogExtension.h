#pragma once
#include "VoltRHI/Extensions/Extension.h"

namespace Volt
{
	enum class LogSeverity
	{
		Trace,
		Log,
		Warning,
		Error,
	};

	class DebugLogExtension : public Extension
	{
	public:
		DebugLogExtension() { m_name = "DebugLog"; };
		~DebugLogExtension() = default;


		void SetTrace(bool flag)	{ SetBitValue(flag, 0); }
		void SetLog(bool flag)		{ SetBitValue(flag, 1); }
		void SetWarning(bool flag)	{ SetBitValue(flag, 2); }
		void SetError(bool flag)	{ SetBitValue(flag, 3); }

		void AssignLogCallback(std::function<void(std::string_view, LogSeverity)>&& callback) { m_logCallback = callback; }


	private:
		void SetBitValue(bool flag, int8_t bit)
		{
			flag ? m_CallbackFilter |= (1 << bit) : m_CallbackFilter &= ~(1 << bit);
		}

		std::function<void(std::string_view, LogSeverity)> m_logCallback;
		
		int8_t m_CallbackFilter = ~0; // packed bools in order | Send Trace | Send Log | Send warning | Send Error |
	};
}
