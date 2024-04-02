#include "circuitpch.h"
#include "CircuitManager.h"

#include "Circuit/Events/CircuitEventTypes.h"

#include "Circuit/Events/Tell/BaseTellEvent.h"
#include "Circuit/Events/Tell/WindowManagementTellEvents.h"

#include "Circuit/Events/Listen/BaseListenEvent.h"
#include "Circuit/Events/Listen/WindowManagementListenEvents.h"

#include "Circuit/Window/CircuitWindow.h"

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

	void CircuitManager::Initialize()
	{
		s_Instance = std::make_unique<CircuitManager>();
		s_Instance->Init();
	}

	void CircuitManager::Init()
	{
		m_TellEvents.push_back(std::make_unique<OpenWindowTellEvent>());
	}

	void CircuitManager::Update()
	{
		if (!m_TellEvents.empty())
		{
			for (long long i = m_TellEvents.size() - 1; i >= 0; i--)
			{
				if (m_TellEvents[i]->IsHandled())
				{
					m_TellEvents.erase(m_TellEvents.begin() + i);
				}
			}
		}
		if (!m_ListenEvents.empty())
		{
			for (long long i = m_ListenEvents.size() - 1; i >= 0; i--)
			{
				HandleListenEvent(m_ListenEvents[i]);
				m_ListenEvents.erase(m_ListenEvents.begin() + i);
			}
		}
	}

	void CircuitManager::AddTellEvent(std::unique_ptr<TellEvent> event)
	{
		m_TellEvents.push_back(std::move(event));
	}

	const std::vector<std::unique_ptr<TellEvent>>& CircuitManager::GetTellEventsToProcess()
	{
		return m_TellEvents;
	}

	CIRCUIT_API void CircuitManager::AddListenEvent(std::unique_ptr<ListenEvent> event)
	{
		m_ListenEvents.push_back(std::move(event));
	}

	void CircuitManager::HandleListenEvent(const std::unique_ptr<ListenEvent>& event)
	{
		switch (event->GetType())
		{
			case CircuitListenEventType::WindowOpened:
			{
				auto openWindowEvent = static_cast<WindowOpenedListenEvent*>(event.get());
				m_Windows[openWindowEvent->GetWindowHandle()] = std::make_unique<CircuitWindow>(openWindowEvent->GetWindowHandle());
			}
			break;

			case CircuitListenEventType::WindowClosed:
			{
				auto closeWindowEvent = static_cast<WindowClosedListenEvent*>(event.get());
				m_Windows.erase(closeWindowEvent->GetWindowHandle());
			}
			break;

			default:
				break;
		}
	}
}
