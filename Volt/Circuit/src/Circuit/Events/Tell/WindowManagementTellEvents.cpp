#include "circuitpch.h"
#include "WindowManagementTellEvents.h"
namespace Circuit
{
	OpenWindowTellEvent::OpenWindowTellEvent(const OpenWindowParams& params)
		: TellEvent(CircuitTellEventType::OpenWindow),
		m_params(params)
	{
	}

	CloseWindowTellEvent::CloseWindowTellEvent(Volt::WindowHandle windowHandle)
		: TellEvent(CircuitTellEventType::CloseWindow)
		, m_WindowHandle(windowHandle)
	{
	}

	SetWindowPositionTellEvent::SetWindowPositionTellEvent(Volt::WindowHandle windowHandle, int x, int y)
		: TellEvent(CircuitTellEventType::SetWindowPosition)
		, m_WindowHandle(windowHandle)
		, m_X(x)
		, m_Y(y)
	{
	}

	SetWindowSizeTellEvent::SetWindowSizeTellEvent(Volt::WindowHandle windowHandle, int width, int height)
		: TellEvent(CircuitTellEventType::SetWindowSize)
		, m_WindowHandle(windowHandle)
		, m_Width(width)
		, m_Height(height)
	{
	}

	SetWindowFocusTellEvent::SetWindowFocusTellEvent(Volt::WindowHandle windowHandle)
		: TellEvent(CircuitTellEventType::SetWindowFocus)
		, m_WindowHandle(windowHandle)
	{
	}
}
