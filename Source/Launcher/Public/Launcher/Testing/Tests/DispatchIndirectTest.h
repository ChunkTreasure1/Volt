#pragma once

#include "Testing/RenderingTestBase.h"

class DispatchIndirectTest : public RenderingTestBase
{
public:
	DispatchIndirectTest();
	~DispatchIndirectTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "DispatchIndirectTest"; }

private:
	RefPtr<Volt::RHI::StorageBuffer> m_buffer;
	RefPtr<Volt::RHI::StorageBuffer> m_commandsBuffer;
	RefPtr<Volt::RHI::StorageBuffer> m_readbackBuffer;

	RefPtr<Volt::RHI::DescriptorTable> m_writeCommandTable;
	RefPtr<Volt::RHI::DescriptorTable> m_indirectDispatchTable;
	RefPtr<Volt::RHI::ComputePipeline> m_writeCommandPipeline;
	RefPtr<Volt::RHI::ComputePipeline> m_indirectDispatchPipeline;
};
