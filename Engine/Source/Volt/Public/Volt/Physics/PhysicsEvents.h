#pragma once

#include <EventSystem/Event.h>

#include <EntitySystem/EntityHelper.h>

namespace Volt
{
	class OnCollisionEnterEvent : public Event
	{
	public:
		OnCollisionEnterEvent(EntityHelper& helperA, EntityHelper& helperB);

		VT_INLINE EntityHelper& GetEntityA() { return m_helperA; }
		VT_INLINE EntityHelper& GetEntityB() { return m_helperA; }

		EVENT_CLASS(OnCollisionEnterEvent, "{7CA1F83F-6349-48C7-92B1-DB6A14C6130D}"_guid);
	private:
		EntityHelper& m_helperA;
		EntityHelper& m_helperB;
	};

	class OnCollisionExitEvent : public Event
	{
	public:
		OnCollisionExitEvent(EntityHelper& helperA, EntityHelper& helperB);

		VT_INLINE EntityHelper& GetEntityA() { return m_helperA; }
		VT_INLINE EntityHelper& GetEntityB() { return m_helperA; }

		EVENT_CLASS(OnCollisionExitEvent, "{B4D16EB6-EEBD-4AE1-A937-119D735E79B3}"_guid)
	private:
		EntityHelper& m_helperA;
		EntityHelper& m_helperB;
	};

	class OnTriggerEnterEvent : public Event
	{
	public:
		OnTriggerEnterEvent(EntityHelper& trigger, EntityHelper& other);

		VT_INLINE EntityHelper& GetTrigger() { return m_trigger; }
		VT_INLINE EntityHelper& GetOther() { return m_other; }

		EVENT_CLASS(OnTriggerEnterEvent, "{83D691E8-A0BD-4439-B185-56D4CB175B22}"_guid);
	private:
		EntityHelper& m_trigger;
		EntityHelper& m_other;
	};

	class OnTriggerExitEvent : public Event
	{
	public:
		OnTriggerExitEvent(EntityHelper& trigger, EntityHelper& other);

		VT_INLINE EntityHelper& GetTrigger() { return m_trigger; }
		VT_INLINE EntityHelper& GetOther() { return m_other; }

		EVENT_CLASS(OnTriggerExitEvent, "{BD19205A-8D60-4EB8-BFB7-6083F95CFAB7}"_guid);
	private:
		EntityHelper& m_trigger;
		EntityHelper& m_other;
	};
}
