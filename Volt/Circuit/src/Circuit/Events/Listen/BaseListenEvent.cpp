#include "circuitpch.h"
#include "BaseListenEvent.h"
namespace Circuit
{
	ListenEvent::ListenEvent(CircuitListenEventType type)
		: m_Type(type)
	{
	}

	CircuitListenEventType ListenEvent::GetType() const
	{
		return m_Type;
	}
}
