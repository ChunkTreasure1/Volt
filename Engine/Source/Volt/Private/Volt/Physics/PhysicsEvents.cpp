#include "vtpch.h"

#include "Volt/Physics/PhysicsEvents.h"

namespace Volt
{
	OnCollisionEnterEvent::OnCollisionEnterEvent(EntityHelper& helperA, EntityHelper& helperB)
		: m_helperA(helperA), m_helperB(helperB)
	{
	}

	OnCollisionExitEvent::OnCollisionExitEvent(EntityHelper& helperA, EntityHelper& helperB)
		: m_helperA(helperA), m_helperB(helperB)
	{
	}

	OnTriggerEnterEvent::OnTriggerEnterEvent(EntityHelper& trigger, EntityHelper& other)
		: m_trigger(trigger), m_other(other)
	{
	}

	OnTriggerExitEvent::OnTriggerExitEvent(EntityHelper& trigger, EntityHelper& other)
		: m_trigger(trigger), m_other(other)
	{
	}
}
