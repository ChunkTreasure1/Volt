#pragma once
#include "Circuit/Events/Tell/BaseTellEvent.h"
#include "Circuit/Events/CircuitEventTypes.h"
#include "Circuit/Window/CircuitWindow.h"

#include <WindowModule/WindowHandle.h>


namespace Circuit
{
	//OpenWindow,
	class OpenWindowTellEvent final : public TellEvent
	{
	public:
		OpenWindowTellEvent(const OpenWindowParams& params);
		~OpenWindowTellEvent() = default;

		const OpenWindowParams& GetParams() const { return m_params; }

	private:
		const OpenWindowParams m_params;
	};

	//CloseWindow
	class CloseWindowTellEvent final : public TellEvent
	{
	public:
		CloseWindowTellEvent(Volt::WindowHandle windowHandle);
		~CloseWindowTellEvent() = default;

		CIRCUIT_API Volt::WindowHandle GetWindowHandle() const { return m_WindowHandle; }

	private:
		const Volt::WindowHandle m_WindowHandle;

	};

	//SetWindowPosition,
	class SetWindowPositionTellEvent final : public TellEvent
	{
	public:
		SetWindowPositionTellEvent(Volt::WindowHandle windowHandle, int x, int y);
		~SetWindowPositionTellEvent() = default;

		CIRCUIT_API Volt::WindowHandle GetWindowHandle() const { return m_WindowHandle; }
		CIRCUIT_API int GetX() const { return m_X; }
		CIRCUIT_API int GetY() const { return m_Y; }

	private:
		const Volt::WindowHandle m_WindowHandle;
		const int m_X;
		const int m_Y;
	};

	//SetWindowSize
	class SetWindowSizeTellEvent final : public TellEvent
	{
	public:
		SetWindowSizeTellEvent(Volt::WindowHandle windowHandle, int width, int height);
		~SetWindowSizeTellEvent() = default;

		CIRCUIT_API Volt::WindowHandle GetWindowHandle() const { return m_WindowHandle; }
		CIRCUIT_API int GetWidth() const { return m_Width; }
		CIRCUIT_API int GetHeight() const { return m_Height; }

	private:
		const Volt::WindowHandle m_WindowHandle;
		const int m_Width;
		const int m_Height;
	};

	//SetWindowFocus,
	class SetWindowFocusTellEvent final : public TellEvent
	{
	public:
		SetWindowFocusTellEvent(Volt::WindowHandle windowHandle);
		~SetWindowFocusTellEvent() = default;

		CIRCUIT_API Volt::WindowHandle GetWindowHandle() const { return m_WindowHandle; }

	private:
		const Volt::WindowHandle m_WindowHandle;
	};
}
