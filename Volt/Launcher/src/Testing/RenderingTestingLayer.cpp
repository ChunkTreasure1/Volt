#include "RenderingTestingLayer.h"

#include "Testing/Tests/SwapchainClearColorTest.h"
#include "Testing/Tests/ComputeWriteToBufferTest.h"
#include "Testing/Tests/ClearBufferToValueTest.h"
#include "Testing/Tests/DispatchIndirectTest.h"
#include "Testing/Tests/DrawIndirectTest.h"
#include "Testing/Tests/PushConstantsTest.h"

#include "Testing/RenderGraphTests/DrawTriangleTest.h"
#include "Testing/RenderGraphTests/ClearCreatedRenderTargetTest.h"
#include "Testing/RenderGraphTests/DispatchComputeShaderTest.h"
#include "Testing/RenderGraphTests/DrawMeshShaderTriangleTest.h"
#include "Testing/RenderGraphTests/DrawMeshShaderMeshTest.h"
#include "Testing/RenderGraphTests/DrawMeshShaderMultipleMeshesTest.h"

#include <LogModule/Log.h>

#include <functional>

using namespace Volt;

void RenderingTestingLayer::OnAttach()
{
	//m_renderingTests.emplace_back(CreateScope<SwapchainClearColorTest>());
	//m_renderingTests.emplace_back(CreateScope<ComputeWriteToBufferTest>());
	//m_renderingTests.emplace_back(CreateScope<ComputeWriteToBufferTest>());
	//m_renderingTests.emplace_back(CreateScope<ClearBufferToValueTest>());
	//m_renderingTests.emplace_back(CreateScope<DispatchIndirectTest>());
	//m_renderingTests.emplace_back(CreateScope<DrawIndirectTest>());
	//m_renderingTests.emplace_back(CreateScope<PushConstantsTest>());

	//m_renderingTests.emplace_back(CreateScope<RG_DrawTriangleTest>());
	//m_renderingTests.emplace_back(CreateScope<RG_ClearCreatedRenderTargetTest>());
	//m_renderingTests.emplace_back(CreateScope<RG_DispatchComputeShaderTest>());
	//m_renderingTests.emplace_back(CreateScope<RG_DrawMeshShaderTriangleTest>());
	//m_renderingTests.emplace_back(CreateScope<RG_DrawMeshShaderMeshTest>());
	m_renderingTests.emplace_back(CreateScope<RG_DrawMeshShaderMultipleMeshesTest>());
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
		if (!test->RunTest())
		{
			VT_LOG(Error, "Test {} failed!", test->GetName());
		}
	}

	return false;
}
