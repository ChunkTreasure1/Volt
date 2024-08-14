#include "circuitpch.h"
#include "CircuitManager.h"

#include "Circuit/Events/CircuitEventTypes.h"

#include "Circuit/Events/Tell/BaseTellEvent.h"
#include "Circuit/Events/Tell/WindowManagementTellEvents.h"

#include "Circuit/Events/Listen/BaseListenEvent.h"
#include "Circuit/Events/Listen/WindowManagementListenEvents.h"

#include "Circuit/Window/CircuitWindow.h"

#include "Circuit/Input/CircuitInput.h"

namespace Circuit
{
	CircuitManager::CircuitManager()
	{
	}

	CircuitManager& CircuitManager::Get()
	{
		assert(s_Instance.get() != nullptr && "CircuitManager instance is null");
		return *s_Instance.get();
	}

	void CircuitManager::Initialize(std::function<void(const TellEvent&)> eventCallback)
	{
		s_Instance = std::make_unique<CircuitManager>();
		s_Instance->Init();
		s_Instance->m_tellEventCallback = eventCallback;
	}

	void CircuitManager::Init()
	{
		CircuitInput::Initialize(
			[this](Circuit::InputEvent& inputEvent)-> bool
		{
			for (auto& pair : m_windows)
			{
				pair.second->OnInputEvent(inputEvent);
			}

			return false;
		});
	}

	void CircuitManager::BroadcastTellEvent(const TellEvent& event)
	{
		m_tellEventCallback(event);
	}

	void CircuitManager::Update()
	{
	}

	void CircuitManager::BroadcastListenEvent(const ListenEvent& event)
	{
		HandleListenEvent(event);
	}

	CircuitWindow& CircuitManager::OpenWindow(OpenWindowParams& params)
	{
		//const size_t startWindowCount = m_windows.size();
		BroadcastTellEvent(OpenWindowTellEvent(params));
		//assert(m_windows.size() == (startWindowCount + 1) && "Failed to open window.");

		return *((--m_windows.end())->second);
	}

	void CircuitManager::HandleListenEvent(const ListenEvent& event)
	{
		switch (event.GetEventType())
		{
			case CircuitListenEventType::WindowOpened:
			{
				const WindowOpenedListenEvent& openWindowEvent = static_cast<const WindowOpenedListenEvent&>(event);
				m_windows[openWindowEvent.GetWindowHandle()] = std::make_unique<CircuitWindow>(openWindowEvent.GetWindowHandle());
			}
			break;

			case CircuitListenEventType::WindowClosed:
			{
				auto closeWindowEvent = static_cast<const WindowClosedListenEvent&>(event);
				m_windows.erase(closeWindowEvent.GetWindowHandle());
			}
			break;

			default:
				break;
		}
	}

	CIRCUIT_API const std::map<Volt::WindowHandle, std::unique_ptr<CircuitWindow>>& CircuitManager::GetWindows()
	{
		return m_windows;
	}
}
