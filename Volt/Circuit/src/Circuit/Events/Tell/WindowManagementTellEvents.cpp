#include "circuitpch.h"
#include "WindowManagementTellEvents.h"
namespace Circuit
{
	OpenWindowTellEvent::OpenWindowTellEvent(const OpenWindowParams& params)
		: TellEvent(CircuitTellEventType::OpenWindow),
		m_params(params)
	{
	}

	CloseWindowTellEvent::CloseWindowTellEvent(InterfaceWindowHandle windowHandle)
		: TellEvent(CircuitTellEventType::CloseWindow)
		, m_WindowHandle(windowHandle)
	{
	}

	SetWindowPositionTellEvent::SetWindowPositionTellEvent(InterfaceWindowHandle windowHandle, int x, int y)
		: TellEvent(CircuitTellEventType::SetWindowPosition)
		, m_WindowHandle(windowHandle)
		, m_X(x)
		, m_Y(y)
	{
	}

	SetWindowSizeTellEvent::SetWindowSizeTellEvent(InterfaceWindowHandle windowHandle, int width, int height)
		: TellEvent(CircuitTellEventType::SetWindowSize)
		, m_WindowHandle(windowHandle)
		, m_Width(width)
		, m_Height(height)
	{
	}

	SetWindowFocusTellEvent::SetWindowFocusTellEvent(InterfaceWindowHandle windowHandle)
		: TellEvent(CircuitTellEventType::SetWindowFocus)
		, m_WindowHandle(windowHandle)
	{
	}
}
