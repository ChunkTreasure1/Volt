#include "Testing/Tests/DispatchIndirectTest.h"

#include <RenderCore/Shader/ShaderMap.h>

#include <RHIModule/Descriptors/DescriptorTable.h>
#include <RHIModule/Buffers/StorageBuffer.h>

using namespace Volt;

constexpr uint32_t GROUP_SIZE = 32;

DispatchIndirectTest::DispatchIndirectTest()
{
	m_writeCommandPipeline = ShaderMap::GetComputePipeline("DispatchIndirectTest_WriteCommand", false);
	m_indirectDispatchPipeline = ShaderMap::GetComputePipeline("DispatchIndirectTest_Dispatch", false);

	m_writeCommandTable = RHI::DescriptorTable::Create({ m_writeCommandPipeline->GetShader() });
	m_indirectDispatchTable = RHI::DescriptorTable::Create({ m_indirectDispatchPipeline->GetShader() });

	m_commandsBuffer = RHI::StorageBuffer::Create<RHI::IndirectDispatchCommand>(1, "Commands Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer);
	m_buffer = RHI::StorageBuffer::Create<uint32_t>(GROUP_SIZE * 2, "Write Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferSrc);
	m_readbackBuffer = RHI::StorageBuffer::Create<uint32_t>(GROUP_SIZE * 2, "Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);

}

DispatchIndirectTest::~DispatchIndirectTest()
{
}

bool DispatchIndirectTest::RunTest()
{
	m_writeCommandTable->SetBufferView("u_commandsBuffer", m_commandsBuffer->GetView(), 0);
	m_indirectDispatchTable->SetBufferView("u_outputBuffer", m_buffer->GetView(), 0);

	m_commandBuffer->Begin();
	m_commandBuffer->BeginMarker("DispatchIndirectTest", { 1.f, 1.f, 1.f, 1.f });

	m_commandBuffer->BindPipeline(m_writeCommandPipeline);
	m_commandBuffer->BindDescriptorTable(m_writeCommandTable);
	m_commandBuffer->Dispatch(1, 1, 1);

	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Buffer;
		barrier.bufferBarrier().srcAccess = RHI::BarrierAccess::ShaderWrite;
		barrier.bufferBarrier().srcStage = RHI::BarrierStage::ComputeShader;
		barrier.bufferBarrier().dstAccess = RHI::BarrierAccess::IndirectArgument;
		barrier.bufferBarrier().dstStage = RHI::BarrierStage::DrawIndirect;
		barrier.bufferBarrier().resource = m_commandsBuffer;
		barrier.bufferBarrier().size = m_commandsBuffer->GetByteSize();

		m_commandBuffer->ResourceBarrier({ barrier });
	}

	m_commandBuffer->BindPipeline(m_indirectDispatchPipeline);
	m_commandBuffer->BindDescriptorTable(m_indirectDispatchTable);
	m_commandBuffer->DispatchIndirect(m_commandsBuffer, 0);

	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Buffer;
		barrier.bufferBarrier().srcAccess = RHI::BarrierAccess::ShaderWrite;
		barrier.bufferBarrier().srcStage = RHI::BarrierStage::ComputeShader;
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

	bool result = true;

	uint32_t* values = m_readbackBuffer->Map<uint32_t>();

	for (uint32_t i = 0; i < GROUP_SIZE * 2; i++)
	{
		if (values[i] != i)
		{
			result = false;
		}
	}

	m_readbackBuffer->Unmap();
	return result;
}
