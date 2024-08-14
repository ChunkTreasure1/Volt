#include "rcpch.h"
#include "GPUReadbackBuffer.h"

#include <RHIModule/Buffers/StorageBuffer.h>

namespace Volt
{
	GPUReadbackBuffer::GPUReadbackBuffer(size_t size)
	{
		m_buffer = RHI::StorageBuffer::Create<uint8_t>(static_cast<uint32_t>(size), "GPU Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);
	}
}
