#pragma once
#include "Circuit/CircuitCoreDefines.h"
#include "Circuit/Window/WindowInterfaceDefines.h"
#include "Circuit/Window/CircuitWindow.h"

#include "Circuit/Events/Tell/BaseTellEvent.h"
#include "Circuit/Events/Listen/BaseListenEvent.h"

#include <vector>
#include <memory>
#include <map>



namespace Circuit
{
	class TellEvent;
	class ListenEvent;
	//class CircuitWindow;
	
	class CircuitManager
	{
	public:
		CircuitManager();
		~CircuitManager() = default;

		CIRCUIT_API static CircuitManager& Get();
		CIRCUIT_API static void Initialize(std::function<void(const TellEvent&)> eventCallback);


		CIRCUIT_API void Update();

		CIRCUIT_API void BroadcastListenEvent(const ListenEvent& event);

		CIRCUIT_API CircuitWindow& OpenWindow(OpenWindowParams& params);

		CIRCUIT_API const std::map<InterfaceWindowHandle, std::unique_ptr<CircuitWindow>>& GetWindows();
	private:
		CIRCUIT_API inline static std::unique_ptr<CircuitManager> s_Instance = nullptr;

		void Init();
		void BroadcastTellEvent(const TellEvent& event);
		void HandleListenEvent(const ListenEvent& event);


		std::function<void(const TellEvent&)> m_tellEventCallback;

		std::map<InterfaceWindowHandle, std::unique_ptr<CircuitWindow>> m_windows;
	};
}
