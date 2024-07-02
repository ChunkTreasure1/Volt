#include "ComputeWriteToBufferTest.h"

#include <Volt/Rendering/Shader/ShaderMap.h>

#include <VoltRHI/Buffers/StorageBuffer.h>

using namespace Volt;

constexpr uint32_t GROUP_SIZE = 32;

ComputeWriteToBufferTest::ComputeWriteToBufferTest()
{
	m_computePipeline = ShaderMap::GetComputePipeline("ComputeWriteToBufferTest");

	RHI::DescriptorTableCreateInfo tableInfo{};
	tableInfo.shader = m_computePipeline->GetShader();

	m_descriptorTable = RHI::DescriptorTable::Create(tableInfo);
	m_buffer = RHI::StorageBuffer::Create<uint32_t>(GROUP_SIZE, "Write Buffer", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPUToCPU);
}

ComputeWriteToBufferTest::~ComputeWriteToBufferTest()
{
	m_descriptorTable = nullptr;
	m_computePipeline = nullptr;
}

void ComputeWriteToBufferTest::RunTest()
{
	m_descriptorTable->SetBufferView("u_outputBuffer", m_buffer->GetView(), 0);

	m_commandBuffer->Begin();

	m_commandBuffer->BindPipeline(m_computePipeline);
	m_commandBuffer->BindDescriptorTable(m_descriptorTable);
	m_commandBuffer->Dispatch(1, 1, 1);
	m_commandBuffer->End();
	m_commandBuffer->ExecuteAndWait();

	uint32_t* values = m_buffer->Map<uint32_t>();
	
	for (uint32_t i = 0; i < GROUP_SIZE; i++)
	{
		uint32_t value = values[i];
		VT_ENSURE(value == i);
	}
	
	m_buffer->Unmap();
}
