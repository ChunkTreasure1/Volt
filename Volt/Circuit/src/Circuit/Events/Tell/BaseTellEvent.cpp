#include "circuitpch.h"
#include "BaseTellEvent.h"
namespace Circuit
{
	TellEvent::TellEvent(CircuitTellEventType type)
		: m_Type(type)
	{
	}

	CircuitTellEventType TellEvent::GetType() const
	{
		return m_Type;
	}

	void TellEvent::MarkHandled()
	{
		m_IsHandled = true;
	}

	CIRCUIT_API bool TellEvent::IsHandled() const
	{
		return m_IsHandled;
	}
}
