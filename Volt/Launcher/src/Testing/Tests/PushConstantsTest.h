#pragma once

#include "Testing/RenderingTestBase.h"

#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Descriptors/DescriptorTable.h>

class PushConstantsTest : public RenderingTestBase
{
public:
	PushConstantsTest();
	~PushConstantsTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "PushConstantsTest"; }

private:
	RefPtr<Volt::RHI::DescriptorTable> m_descriptorTable;
	RefPtr<Volt::RHI::ComputePipeline> m_computePipeline;
	RefPtr<Volt::RHI::StorageBuffer> m_buffer;
	RefPtr<Volt::RHI::StorageBuffer> m_readbackBuffer;
};
