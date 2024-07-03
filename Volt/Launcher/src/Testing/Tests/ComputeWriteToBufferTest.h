#pragma once

#include "Testing/RenderingTestBase.h"

#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>

class ComputeWriteToBufferTest : public RenderingTestBase
{
public: 
	ComputeWriteToBufferTest();
	~ComputeWriteToBufferTest() override;

	bool RunTest();
	std::string GetName() const override { return "ComputeWriteToBufferTest"; }

private:
	RefPtr<Volt::RHI::DescriptorTable> m_descriptorTable;
	RefPtr<Volt::RHI::ComputePipeline> m_computePipeline;
	RefPtr<Volt::RHI::StorageBuffer> m_buffer;
	RefPtr<Volt::RHI::StorageBuffer> m_readbackBuffer;
};
