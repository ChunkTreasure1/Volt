#pragma once
#include "Circuit/Config.h"
#include "Circuit/Events/Listen/BaseListenEvent.h"
#include <WindowModule/WindowHandle.h>

namespace Circuit
{
	class WindowOpenedListenEvent : public ListenEvent
	{
	public:
		CIRCUIT_API WindowOpenedListenEvent(Volt::WindowHandle windowHandle);
		~WindowOpenedListenEvent() = default;

		const Volt::WindowHandle& GetWindowHandle() const { return m_WindowHandle; }
		LISTEN_EVENT_CLASS_TYPE(CircuitListenEventType::WindowOpened)
	private:
		const Volt::WindowHandle m_WindowHandle;
	};

	class WindowClosedListenEvent : public ListenEvent
	{
	public:
		CIRCUIT_API WindowClosedListenEvent(Volt::WindowHandle windowHandle);
		~WindowClosedListenEvent() = default;

		const Volt::WindowHandle& GetWindowHandle() const { return m_WindowHandle; }
		LISTEN_EVENT_CLASS_TYPE(CircuitListenEventType::WindowClosed)

	private:
		const Volt::WindowHandle m_WindowHandle;
	};

	class WindowResizedListenEvent : public ListenEvent
	{
	public:
		CIRCUIT_API WindowResizedListenEvent(Volt::WindowHandle windowHandle, const glm::vec2& size);
		~WindowResizedListenEvent() = default;

		const Volt::WindowHandle& GetWindowHandle() const { return m_WindowHandle; }
		const glm::vec2& GetSize() const { return m_size; }
		LISTEN_EVENT_CLASS_TYPE(CircuitListenEventType::WindowResized)

	private:
		const Volt::WindowHandle m_WindowHandle;
		const glm::vec2 m_size;
	};
}
