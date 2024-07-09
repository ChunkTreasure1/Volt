#include "ComputeWriteToBufferTest.h"

#include <Volt/Rendering/Shader/ShaderMap.h>

#include <VoltRHI/Buffers/StorageBuffer.h>

using namespace Volt;

constexpr uint32_t GROUP_SIZE = 32;

ComputeWriteToBufferTest::ComputeWriteToBufferTest()
{
	m_computePipeline = ShaderMap::GetComputePipeline("ComputeWriteToBufferTest", false);

	RHI::DescriptorTableCreateInfo tableInfo{};
	tableInfo.shader = m_computePipeline->GetShader();

	m_descriptorTable = RHI::DescriptorTable::Create(tableInfo);
	m_buffer = RHI::StorageBuffer::Create<uint32_t>(GROUP_SIZE, "Write Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferSrc, RHI::MemoryUsage::GPU);
	m_readbackBuffer = RHI::StorageBuffer::Create<uint32_t>(GROUP_SIZE, "Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);
}

ComputeWriteToBufferTest::~ComputeWriteToBufferTest()
{
	m_descriptorTable = nullptr;
	m_computePipeline = nullptr;
}

bool ComputeWriteToBufferTest::RunTest()
{
	m_descriptorTable->SetBufferView("u_outputBuffer", m_buffer->GetView(), 0);

	m_commandBuffer->Begin();
	m_commandBuffer->BeginMarker("ComputeWriteToBufferTest", { 1.f, 1.f, 1.f, 1.f });

	m_commandBuffer->BindPipeline(m_computePipeline);
	m_commandBuffer->BindDescriptorTable(m_descriptorTable);
	m_commandBuffer->Dispatch(1, 1, 1);

	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Buffer;
		barrier.bufferBarrier().srcAccess = RHI::BarrierAccess::ShaderWrite;
		barrier.bufferBarrier().srcStage = RHI::BarrierStage::ComputeShader;
		barrier.bufferBarrier().dstAccess = RHI::BarrierAccess::TransferSource;
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

	for (uint32_t i = 0; i < GROUP_SIZE; i++)
	{
		if (values[i] != i)
		{
			result = false;
		}
	}

	m_readbackBuffer->Unmap();
	return result;
}
