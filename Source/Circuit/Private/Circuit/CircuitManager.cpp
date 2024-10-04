#include "circuitpch.h"
#include "CircuitManager.h"

#include "Circuit/Window/CircuitWindow.h"

#include "Circuit/Widgets/SliderWidget.h"
#include "Circuit/Widgets/TextWidget.h"
#include "Circuit/Widgets/ButtonWidget.h"

#include <WindowModule/WindowManager.h>
#include <WindowModule/Events/WindowEvents.h>
#include <WindowModule/Window.h>

#include <CoreUtilities/Delegates/Delegate.h>

#include <LogModule/Log.h>

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

		static float sliderValue = 50.f;
		m_windows[Volt::WindowManager::Get().GetMainWindowHandle()]->SetWidget(
			CreateWidget(SliderWidget)
			.X(100)
			.Y(100)
			.MinValue(0)
			.MaxValue(100)
			.Value_Lambda([]() {return sliderValue; })
			.OnValueChanged_Lambda([](float newValue) 
		{
			sliderValue = newValue; 
		})
		);
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

	int32_t CircuitManager::TestingStaticDelegates(float aParameter)
	{
		VT_LOG(Warning, "I WAS CALLED FROM A STATIC DELEGATE!!! FloatParameter: {0}", aParameter);

		return static_cast<int32_t>(aParameter * 2);
	}

	int32_t CircuitManager::TestingRawDelegates(float aParameter)
	{
		VT_LOG(Warning, "I WAS CALLED FROM A RAW DELEGATE!!! FloatParameter: {0}, WindowCount: {1}", aParameter, m_windows.size());

		return static_cast<int32_t>(aParameter / 2);
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
