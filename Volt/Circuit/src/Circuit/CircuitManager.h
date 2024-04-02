#pragma once
#include "Circuit/CircuitCoreDefines.h"
#include "Circuit/Window/WindowInterfaceDefines.h"
#include "Circuit/Window/CircuitWindow.h"
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
		CIRCUIT_API static void Initialize();

		CIRCUIT_API void Update();

		CIRCUIT_API const std::vector<std::unique_ptr<TellEvent>>& GetTellEventsToProcess();

		CIRCUIT_API void AddListenEvent(std::unique_ptr<ListenEvent> event);
	private:
		CIRCUIT_API inline static std::unique_ptr<CircuitManager> s_Instance = nullptr;

		void Init();
		void AddTellEvent(std::unique_ptr<TellEvent> event);
		void HandleListenEvent(const std::unique_ptr<ListenEvent>& event);


		std::vector<std::unique_ptr<TellEvent>> m_TellEvents;
		std::vector<std::unique_ptr<ListenEvent>> m_ListenEvents;

		std::map<InterfaceWindowHandle, std::unique_ptr<CircuitWindow>> m_Windows;
	};
}
