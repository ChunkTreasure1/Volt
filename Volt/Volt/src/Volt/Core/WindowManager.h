#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/WindowHandle.h"

#include <unordered_map>

namespace Volt
{
	class Window;
	struct WindowProperties;

	class WindowManager
	{
	public:
		const WindowHandle CreateNewWindow(const WindowProperties& windowProperties);
		void DestroyWindow(const WindowHandle handle);

		void BeginFrame();
		void Present();

		Window& GetWindow(const WindowHandle handle) const;

	private:
		std::unordered_map<WindowHandle, Scope<Window>> m_windows;
	};
}
