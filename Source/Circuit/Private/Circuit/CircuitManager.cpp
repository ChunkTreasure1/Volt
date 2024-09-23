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

		m_windows[Volt::WindowManager::Get().GetMainWindowHandle()]->SetWidget(
			CreateWidget(Circuit::TextWidget)
		.X(100)
		.Y(100)
			.Text("This is some text!")
		);

		int32_t capturedLocalInt = 10;
		int32_t copiedLocalInt = 5;
		Volt::Delegate<int32_t(float)> testLambdaDelegate = Volt::Delegate<int32_t(float)>::CreateLambda([&capturedLocalInt, copied = copiedLocalInt, this](float aParam)
		{
			VT_LOG(Warning, "I WAS CALLED FROM A LAMBDA DELEGATE!!! capturedLocalInt: {0}, copiedLocalInt: {1}, FloatParameter: {2}, WindowCount from this: {3}", capturedLocalInt, copied, aParam, m_windows.size());

			return capturedLocalInt * copied;
		});
		int32_t lambdaResult = testLambdaDelegate.Execute(52.5f);
		VT_LOG(Warning, "Result from lambda delegate: {0}", lambdaResult);

		Volt::Delegate<int32_t(float)> testStaticDelegate = Volt::Delegate<int32_t(float)>::CreateStatic(&CircuitManager::TestingStaticDelegates);
		int32_t staticResult = testStaticDelegate.Execute(52.5f);
		VT_LOG(Warning, "Result from static delegate: {0}", staticResult);

		Volt::Delegate<int32_t(float)> testRawDelegate = Volt::Delegate<int32_t(float)>::CreateRaw(this, &CircuitManager::TestingRawDelegates);
		int32_t rawResult = testRawDelegate.Execute(52.5f);
		VT_LOG(Warning, "Result from Raw delegate: {0}", rawResult);




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
