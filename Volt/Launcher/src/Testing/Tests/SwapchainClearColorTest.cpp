#include "SwapchainClearColorTest.h"

#include <Volt/Core/Application.h>


using namespace Volt;

SwapchainClearColorTest::SwapchainClearColorTest()
{
}

SwapchainClearColorTest::~SwapchainClearColorTest()
{
}

bool SwapchainClearColorTest::RunTest()
{
	auto& swapchain = Application::Get().GetWindow().GetSwapchain();

	m_commandBuffer->Begin();

	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Image;
		barrier.imageBarrier().srcAccess = RHI::BarrierAccess::None;
		barrier.imageBarrier().srcStage = RHI::BarrierStage::All;
		barrier.imageBarrier().srcLayout = RHI::ImageLayout::Undefined;
		barrier.imageBarrier().dstAccess = RHI::BarrierAccess::RenderTarget;
		barrier.imageBarrier().dstStage = RHI::BarrierStage::RenderTarget;
		barrier.imageBarrier().dstLayout = RHI::ImageLayout::RenderTarget;
		barrier.imageBarrier().resource = swapchain.GetCurrentImage();

		m_commandBuffer->ResourceBarrier({ barrier });
	}

	RHI::AttachmentInfo attachment{};
	attachment.view = swapchain.GetCurrentImage()->GetView();
	attachment.clearMode = RHI::ClearMode::Clear;
	attachment.clearColor = { 1.f, 0.f, 0.f, 1.f };

	RHI::RenderingInfo renderingInfo{};
	renderingInfo.colorAttachments = { attachment };
	renderingInfo.renderArea.extent.width = swapchain.GetWidth();
	renderingInfo.renderArea.extent.height = swapchain.GetHeight();

	m_commandBuffer->BeginRendering(renderingInfo);
	m_commandBuffer->EndRendering();

	m_commandBuffer->End();
	m_commandBuffer->Execute();

	return true;
}
