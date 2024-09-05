#include "Testing/Tests/ClearBufferToValueTest.h"

#include <RHIModule/Buffers/StorageBuffer.h>

using namespace Volt;

constexpr uint32_t BUFFER_SIZE = 100;

ClearBufferToValueTest::ClearBufferToValueTest()
{
	m_buffer = RHI::StorageBuffer::Create<uint32_t>(BUFFER_SIZE, "Write Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferSrc, RHI::MemoryUsage::GPU);
	m_readbackBuffer = RHI::StorageBuffer::Create<uint32_t>(BUFFER_SIZE, "Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);
}

ClearBufferToValueTest::~ClearBufferToValueTest()
{
}

bool ClearBufferToValueTest::RunTest()
{
	constexpr uint32_t ITERATIONS = 10;

	bool result = true;

	for (uint32_t i = 0; i < ITERATIONS; i++)
	{
		m_commandBuffer->Begin();
		m_commandBuffer->BeginMarker("ClearBufferToValueTest", { 1.f, 1.f, 1.f, 1.f });

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

		m_commandBuffer->ClearBuffer(m_buffer, i);

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
	
		uint32_t* values = m_readbackBuffer->Map<uint32_t>();

		for (uint32_t j = 0; j < BUFFER_SIZE; j++)
		{
			if (values[j] != i)
			{
				result = false;
			}
		}

		m_readbackBuffer->Unmap();
	}

	return result;
}
