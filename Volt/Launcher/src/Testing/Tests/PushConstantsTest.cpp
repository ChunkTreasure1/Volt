#include "PushConstantsTest.h"

#include <Volt/Rendering/Shader/ShaderMap.h>

#include <VoltRHI/Buffers/StorageBuffer.h>

using namespace Volt;

PushConstantsTest::PushConstantsTest()
{
	m_computePipeline = ShaderMap::GetComputePipeline("PushConstantsTest", false);

	RHI::DescriptorTableCreateInfo tableInfo{};
	tableInfo.shader = m_computePipeline->GetShader();

	m_descriptorTable = RHI::DescriptorTable::Create(tableInfo);
	m_buffer = RHI::StorageBuffer::Create<uint32_t>(1, "Write Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferSrc, RHI::MemoryUsage::GPU);
	m_readbackBuffer = RHI::StorageBuffer::Create<uint32_t>(1, "Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);
}

PushConstantsTest::~PushConstantsTest()
{
}

bool PushConstantsTest::RunTest()
{
	m_descriptorTable->SetBufferView("u_outputBuffer", m_buffer->GetView(), 0);

	constexpr uint32_t ITERATIONS = 10;

	bool result = true;

	for (uint32_t i = 0; i < ITERATIONS; i++)
	{
		m_commandBuffer->Begin();
		m_commandBuffer->BeginMarker("PushConstantsTest", { 1.f, 1.f, 1.f, 1.f });

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Buffer;
			barrier.bufferBarrier().srcAccess = RHI::BarrierAccess::None;
			barrier.bufferBarrier().srcStage = RHI::BarrierStage::AllGraphics;
			barrier.bufferBarrier().dstAccess = RHI::BarrierAccess::CopyDest;
			barrier.bufferBarrier().dstStage = RHI::BarrierStage::Copy;
			barrier.bufferBarrier().resource = m_buffer;
			barrier.bufferBarrier().size = m_buffer->GetByteSize();

			m_commandBuffer->ResourceBarrier({ barrier });
		}

		m_commandBuffer->BindPipeline(m_computePipeline);
		m_commandBuffer->BindDescriptorTable(m_descriptorTable);
		m_commandBuffer->PushConstants(&i, sizeof(uint32_t), 0);
		m_commandBuffer->Dispatch(1, 1, 1);

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Buffer;
			barrier.bufferBarrier().srcAccess = RHI::BarrierAccess::CopyDest;
			barrier.bufferBarrier().srcStage = RHI::BarrierStage::Copy;
			barrier.bufferBarrier().dstAccess = RHI::BarrierAccess::CopySource;
			barrier.bufferBarrier().dstStage = RHI::BarrierStage::Copy;
			barrier.bufferBarrier().resource = m_buffer;
			barrier.bufferBarrier().size = m_buffer->GetByteSize();

			m_commandBuffer->ResourceBarrier({ barrier });
		}

		m_commandBuffer->CopyBufferRegion(m_buffer->GetAllocation(), 0, m_readbackBuffer->GetAllocation(), 0, m_readbackBuffer->GetByteSize());
		m_commandBuffer->EndMarker();
		m_commandBuffer->End();
		m_commandBuffer->ExecuteAndWait();

		uint32_t* value = m_readbackBuffer->Map<uint32_t>();
		if (*value != i)
		{
			result = false;
		}
		m_readbackBuffer->Unmap();
	}

	return result;
}
