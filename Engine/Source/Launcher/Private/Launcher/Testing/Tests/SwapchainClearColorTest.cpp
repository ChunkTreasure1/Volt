#include "Testing/Tests/SwapchainClearColorTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Rendering/Renderer.h>

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

using namespace Volt;

SwapchainClearColorTest::SwapchainClearColorTest()
	: m_commandBufferSet(Renderer::GetFramesInFlight())
{
}

SwapchainClearColorTest::~SwapchainClearColorTest()
{
}

bool SwapchainClearColorTest::RunTest()
{
	auto& swapchain = Volt::WindowManager::Get().GetMainWindow().GetSwapchain();

	auto commandBuffer = m_commandBufferSet.IncrementAndGetCommandBuffer();

	commandBuffer->Begin();
	commandBuffer->BeginMarker("SwapchainClearColorTest", { 1.f });

	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Image;
		barrier.imageBarrier().srcAccess = RHI::BarrierAccess::None;
		barrier.imageBarrier().srcStage = RHI::BarrierStage::RenderTarget;
		barrier.imageBarrier().srcLayout = RHI::ImageLayout::Undefined;
		barrier.imageBarrier().dstAccess = RHI::BarrierAccess::RenderTarget;
		barrier.imageBarrier().dstStage = RHI::BarrierStage::RenderTarget;
		barrier.imageBarrier().dstLayout = RHI::ImageLayout::RenderTarget;
		barrier.imageBarrier().resource = swapchain.GetCurrentImage();

		commandBuffer->ResourceBarrier({ barrier });
	}

	RHI::AttachmentInfo attachment{};
	attachment.view = swapchain.GetCurrentImage()->GetView();
	attachment.clearMode = RHI::ClearMode::Clear;
	attachment.clearColor = { 1.f, 0.f, 0.f, 1.f };

	RHI::RenderingInfo renderingInfo{};
	renderingInfo.colorAttachments = { attachment };
	renderingInfo.renderArea.extent.width = swapchain.GetWidth();
	renderingInfo.renderArea.extent.height = swapchain.GetHeight();

	commandBuffer->BeginRendering(renderingInfo);
	commandBuffer->EndRendering();

	commandBuffer->EndMarker();
	commandBuffer->End();
	commandBuffer->Execute();

	return true;
}
