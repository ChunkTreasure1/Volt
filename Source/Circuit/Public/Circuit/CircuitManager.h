#pragma once
#include "Circuit/Config.h"
#include "Circuit/Window/CircuitWindow.h"

#include <EventSystem/EventListener.h>

#include <WindowModule/WindowHandle.h>

#include <vector>
#include <memory>
#include <map>


namespace Volt
{
	class WindowRenderEvent;

	class Window;
}

namespace Circuit
{
	class CircuitManager : Volt::EventListener
	{
	public:
		CircuitManager();
		~CircuitManager() = default;

		CIRCUIT_API static CircuitManager& Get();
		CIRCUIT_API static void Initialize();

		CIRCUIT_API void Update();

		//CIRCUIT_API CircuitWindow& OpenWindow(OpenWindowParams& params);  
	private:
		CIRCUIT_API inline static std::unique_ptr<CircuitManager> s_Instance = nullptr;

		void Init();
		void RegisterEventListeners();

		bool OnRenderEvent(Volt::WindowRenderEvent& e);

		void RegisterWindow(Volt::WindowHandle handle);

	private:
		std::map<Volt::WindowHandle, Scope<CircuitWindow>> m_windows;
	};
}
