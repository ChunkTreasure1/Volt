#include "DrawIndirectTest.h"

#include <Volt/Rendering/Shader/ShaderMap.h>
#include <Volt/Core/Application.h>

#include <RHIModule/Descriptors/DescriptorTable.h>
#include <RHIModule/Buffers/StorageBuffer.h>

#include <RHIModule/Utility/ResourceUtility.h>

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

using namespace Volt;

DrawIndirectTest::DrawIndirectTest()
{
	RHI::RenderPipelineCreateInfo pipelineInfo{};
	pipelineInfo.shader = ShaderMap::Get("DrawIndirectTest");
	m_renderPipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

	//m_descriptorTable = RHI::DescriptorTable::Create({ m_renderPipeline->GetShader(), false });
	m_commandsBuffer = RHI::StorageBuffer::Create<RHI::IndirectDrawCommand>(1, "Commands Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::CPUToGPU);
}

DrawIndirectTest::~DrawIndirectTest()
{
}

bool DrawIndirectTest::RunTest()
{
	auto& swapchain = Volt::WindowManager::Get().GetMainWindow().GetSwapchain();

	// Setup command
	{
		RHI::IndirectDrawCommand* cmd = m_commandsBuffer->Map<RHI::IndirectDrawCommand>();

		cmd->firstInstance = 0;
		cmd->firstVertex = 0;
		cmd->instanceCount = 1;
		cmd->vertexCount = 3;

		m_commandsBuffer->Unmap();
	}

	m_commandBuffer->Begin();
	m_commandBuffer->BeginMarker("DrawIndirectTest", { 1.f, 1.f, 1.f, 1.f });

	RHI::AttachmentInfo attachment{};
	attachment.view = swapchain.GetCurrentImage()->GetView();
	attachment.clearMode = RHI::ClearMode::Load;

	RHI::RenderingInfo renderingInfo{};
	renderingInfo.colorAttachments = { attachment };
	renderingInfo.renderArea.extent.width = swapchain.GetWidth();
	renderingInfo.renderArea.extent.height = swapchain.GetHeight();

	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Buffer;
		barrier.bufferBarrier().srcAccess = RHI::BarrierAccess::None;
		barrier.bufferBarrier().srcStage = RHI::BarrierStage::None;
		barrier.bufferBarrier().dstAccess = RHI::BarrierAccess::IndirectArgument;
		barrier.bufferBarrier().dstStage = RHI::BarrierStage::DrawIndirect;
		barrier.bufferBarrier().resource = m_commandsBuffer;
		barrier.bufferBarrier().size = m_commandsBuffer->GetByteSize();

		m_commandBuffer->ResourceBarrier({ barrier });
	}

	RHI::Viewport viewport;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	viewport.width = static_cast<float>(swapchain.GetWidth());
	viewport.height = static_cast<float>(swapchain.GetHeight());

	m_commandBuffer->BeginRendering(renderingInfo);
	m_commandBuffer->SetScissors({ renderingInfo.renderArea });
	m_commandBuffer->SetViewports({ viewport });

	//m_commandBuffer->BindDescriptorTable(m_descriptorTable);
	m_commandBuffer->BindPipeline(m_renderPipeline);
	m_commandBuffer->DrawIndirect(m_commandsBuffer, 0, 1, sizeof(RHI::IndirectDrawCommand));
	m_commandBuffer->EndRendering();

	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Image;
		RHI::ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), swapchain.GetCurrentImage());

		barrier.imageBarrier().dstAccess = RHI::BarrierAccess::None;
		barrier.imageBarrier().dstStage = RHI::BarrierStage::AllGraphics;
		barrier.imageBarrier().dstLayout = RHI::ImageLayout::Present;
		barrier.imageBarrier().resource = swapchain.GetCurrentImage();

		m_commandBuffer->ResourceBarrier({ barrier });
	}

	m_commandBuffer->EndMarker();
	m_commandBuffer->End();
	m_commandBuffer->ExecuteAndWait();

	return true;
}
