#include "circuitpch.h"
#include "WindowManagementListenEvents.h"
#include "Circuit/Events/CircuitEventTypes.h"
namespace Circuit
{
	WindowOpenedListenEvent::WindowOpenedListenEvent(Volt::WindowHandle windowHandle)
		: ListenEvent()
		, m_WindowHandle(windowHandle)
	{
	}

	WindowClosedListenEvent::WindowClosedListenEvent(Volt::WindowHandle windowHandle)
		: ListenEvent()
		, m_WindowHandle(windowHandle)
	{
	}
	WindowResizedListenEvent::WindowResizedListenEvent(Volt::WindowHandle windowHandle, const glm::vec2& size)
		: ListenEvent()
		, m_WindowHandle(windowHandle)
		, m_size(size)
	{
	}
}
