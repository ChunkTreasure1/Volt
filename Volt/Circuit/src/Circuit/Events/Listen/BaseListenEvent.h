#pragma once
#include "Circuit/CircuitCoreDefines.h"

namespace Circuit
{
	enum class CircuitListenEventType : unsigned int;
	class ListenEvent
	{
	public:
		CIRCUIT_API ListenEvent(CircuitListenEventType type);
		virtual ~ListenEvent() = default;

		CIRCUIT_API CircuitListenEventType GetType() const;

	private:
		const CircuitListenEventType m_Type;
	};
}
