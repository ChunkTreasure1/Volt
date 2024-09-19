#include "circuitpch.h"
#include "CircuitManager.h"

#include "Circuit/Window/CircuitWindow.h"

#include "Circuit/Widgets/SliderWidget.h"
#include "Circuit/Widgets/TextWidget.h"

#include <WindowModule/WindowManager.h>
#include <WindowModule/Events/WindowEvents.h>
#include <WindowModule/Window.h>

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
		RegisterEventListeners();

		RegisterWindow(Volt::WindowManager::Get().GetMainWindowHandle());

		m_windows[Volt::WindowManager::Get().GetMainWindowHandle()]->SetWidget(
			CreateWidget(Circuit::TextWidget)
		.X(100)
		.Y(100)
		.Text("Hej Ivar!")
		);


		//CreateWidget(Circuit::SliderWidget)
		//	.Max(100)
		//	.Min(0)
		//	.Value(50.f);


		//CreateWidget(Circuit::SliderWidget)
		//	.Max(this, &SliderWidget::GetMaxValue)
		//	.Min(this, &SliderWidget::GetMinValue)
		//	.Value(this, &SliderWidget::GetValue);

		//.Value_lambda([]() 
		//	{
		//	return 100.f;
		//	})
	}

	void CircuitManager::RegisterEventListeners()
	{
		RegisterListener<Volt::WindowRenderEvent>(VT_BIND_EVENT_FN(CircuitManager::OnRenderEvent));
	}

	bool CircuitManager::OnRenderEvent(Volt::WindowRenderEvent& e)
	{
		for (auto& [windowHandle, window] : m_windows)
		{
			window->OnRender();
		}

		return false;
	}

	void CircuitManager::RegisterWindow(Volt::WindowHandle handle)
	{
		m_windows.emplace(handle, CreateScope<CircuitWindow>(handle));
	}

	void CircuitManager::Update()
	{
	}

	//CircuitWindow& CircuitManager::OpenWindow(OpenWindowParams& params)
	//{
	//	//const size_t startWindowCount = m_windows.size();
	//	//BroadcastTellEvent(OpenWindowTellEvent(params));
	//	//assert(m_windows.size() == (startWindowCount + 1) && "Failed to open window.");

	//	return *((--m_windows.end())->second);
	//}
}
