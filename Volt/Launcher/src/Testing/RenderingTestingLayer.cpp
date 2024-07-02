#include "RenderingTestingLayer.h"

#include "Testing/Tests/SwapchainClearColorTest.h"
#include "Testing/Tests/ComputeWriteToBufferTest.h"

#include <functional>

using namespace Volt;

void RenderingTestingLayer::OnAttach()
{
	m_renderingTests.emplace_back(CreateScope<SwapchainClearColorTest>());
	m_renderingTests.emplace_back(CreateScope<ComputeWriteToBufferTest>());
}

void RenderingTestingLayer::OnDetach()
{
	m_renderingTests.clear();
}

void RenderingTestingLayer::OnEvent(Volt::Event& e)
{
	EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<AppRenderEvent>(VT_BIND_EVENT_FN(RenderingTestingLayer::OnRenderEvent));
}

bool RenderingTestingLayer::OnRenderEvent(Volt::AppRenderEvent& e)
{
	for (const auto& test : m_renderingTests)
	{
		test->RunTest();
	}

	return false;
}
