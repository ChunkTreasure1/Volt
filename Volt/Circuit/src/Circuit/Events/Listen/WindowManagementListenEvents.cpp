#include "circuitpch.h"
#include "WindowManagementListenEvents.h"
#include "Circuit/Events/CircuitEventTypes.h"
namespace Circuit
{
	WindowOpenedListenEvent::WindowOpenedListenEvent(InterfaceWindowHandle windowHandle)
		: ListenEvent()
		, m_WindowHandle(windowHandle)
	{
	}

	WindowClosedListenEvent::WindowClosedListenEvent(InterfaceWindowHandle windowHandle)
		: ListenEvent()
		, m_WindowHandle(windowHandle)
	{
	}
	WindowResizedListenEvent::WindowResizedListenEvent(InterfaceWindowHandle windowHandle, const glm::vec2& size)
		: ListenEvent()
		, m_WindowHandle(windowHandle)
		, m_size(size)
	{
	}
}
