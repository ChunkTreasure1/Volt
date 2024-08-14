#pragma once
#include "Circuit/Config.h"

namespace Circuit
{
	enum class CircuitTellEventType : unsigned int;
	class TellEvent
	{
	public:
		TellEvent(CircuitTellEventType type);
		virtual ~TellEvent() = default;

		CIRCUIT_API CircuitTellEventType GetType() const;
		CIRCUIT_API void MarkHandled();
		CIRCUIT_API bool IsHandled() const;

	private:
		const CircuitTellEventType m_Type;
		bool m_IsHandled = false;
	};
}
