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

		const InterfaceWindowHandle& GetWindowHandle() const { return m_WindowHandle; }
		LISTEN_EVENT_CLASS_TYPE(CircuitListenEventType::WindowOpened)
	private:
		const InterfaceWindowHandle m_WindowHandle;
	};

	class WindowClosedListenEvent : public ListenEvent
	{
	public:
		CIRCUIT_API WindowClosedListenEvent(InterfaceWindowHandle windowHandle);
		~WindowClosedListenEvent() = default;

		const InterfaceWindowHandle& GetWindowHandle() const { return m_WindowHandle; }
		LISTEN_EVENT_CLASS_TYPE(CircuitListenEventType::WindowClosed)

	private:
		const InterfaceWindowHandle m_WindowHandle;
	};

	class WindowResizedListenEvent : public ListenEvent
	{
	public:
		CIRCUIT_API WindowResizedListenEvent(InterfaceWindowHandle windowHandle, const glm::vec2& size);
		~WindowResizedListenEvent() = default;

		const InterfaceWindowHandle& GetWindowHandle() const { return m_WindowHandle; }
		const glm::vec2& GetSize() const { return m_size; }
		LISTEN_EVENT_CLASS_TYPE(CircuitListenEventType::WindowResized)

	private:
		const InterfaceWindowHandle m_WindowHandle;
		const glm::vec2 m_size;
	};
}
