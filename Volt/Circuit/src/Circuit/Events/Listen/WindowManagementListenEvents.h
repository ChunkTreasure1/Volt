#pragma once
#include "Circuit/CircuitCoreDefines.h"
#include "Circuit/Events/Listen/BaseListenEvent.h"
#include "Circuit/Window/WindowInterfaceDefines.h"

namespace Circuit
{
	class WindowOpenedListenEvent : public ListenEvent
	{
	public:
		CIRCUIT_API WindowOpenedListenEvent(InterfaceWindowHandle windowHandle);
		~WindowOpenedListenEvent() = default;

		InterfaceWindowHandle GetWindowHandle() const { return m_WindowHandle; }

	private:
		const InterfaceWindowHandle m_WindowHandle;
	};

	class WindowClosedListenEvent : public ListenEvent
	{
	public:
		CIRCUIT_API WindowClosedListenEvent(InterfaceWindowHandle windowHandle);
		~WindowClosedListenEvent() = default;

		InterfaceWindowHandle GetWindowHandle() const { return m_WindowHandle; }

	private:
		const InterfaceWindowHandle m_WindowHandle;
	};
}
