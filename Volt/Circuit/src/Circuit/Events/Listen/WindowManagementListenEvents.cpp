#include "circuitpch.h"
#include "WindowManagementListenEvents.h"
#include "Circuit/Events/CircuitEventTypes.h"
namespace Circuit
{
	WindowOpenedListenEvent::WindowOpenedListenEvent(InterfaceWindowHandle windowHandle)
		: ListenEvent(CircuitListenEventType::WindowOpened)
		, m_WindowHandle(windowHandle)
	{
	}

	WindowClosedListenEvent::WindowClosedListenEvent(InterfaceWindowHandle windowHandle)
		: ListenEvent(CircuitListenEventType::WindowClosed)
		, m_WindowHandle(windowHandle)
	{
	}
	WindowResizedListenEvent::WindowResizedListenEvent(InterfaceWindowHandle windowHandle, const glm::vec2& size)
		: ListenEvent(CircuitListenEventType::WindowResized)
		, m_WindowHandle(windowHandle)
		, m_size(size)
	{
	}
}
