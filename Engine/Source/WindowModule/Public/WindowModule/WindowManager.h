#pragma once

#include "WindowModule/WindowHandle.h"
#include "WindowModule/WindowProperties.h"

#include "WindowModule/Config.h"

#include <SubSystem/SubSystem.h>
#include <CoreUtilities/Core.h>

#include <unordered_map>

namespace Volt
{
	class Window;

	class WINDOWMODULE_API WindowManager : public SubSystem
	{
	public:
		WindowManager();
		~WindowManager();

		WindowManager(const WindowManager&) = delete;
		WindowManager& operator=(const WindowManager&) = delete;

		static void InitializeGLFW();
		static void ShutdownGLFW();

		void Initialize() override;
		void Shutdown() override;

		void CreateMainWindow(const WindowProperties& windowProperties);
		void DestroyMainWindow();

		const WindowHandle CreateNewWindow(const WindowProperties& windowProperties);
		void DestroyWindow(const WindowHandle handle);

		void BeginFrame();
		void Render();
		void Present();

		WindowHandle GetMainWindowHandle() const;

		Window& GetMainWindow() const;
		Window& GetWindow(const WindowHandle handle) const;

		static WindowManager& Get();

		VT_DECLARE_SUBSYSTEM("{DD8C1066-AA16-40C6-929D-282F15D11AC2}"_guid);

	private:
		inline static WindowManager* s_instance = nullptr;

		WindowHandle m_mainWindowHandle;
		std::unordered_map<WindowHandle, Scope<Window>> m_windows;
	};
}
