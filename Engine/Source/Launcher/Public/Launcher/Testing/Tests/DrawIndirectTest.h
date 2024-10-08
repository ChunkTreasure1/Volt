#pragma once

#include "Testing/RenderingTestBase.h"

class DrawIndirectTest : public RenderingTestBase
{
public:
	DrawIndirectTest();
	~DrawIndirectTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "DrawIndirectTest"; }

private:
	RefPtr<Volt::RHI::StorageBuffer> m_commandsBuffer;

	RefPtr<Volt::RHI::DescriptorTable> m_descriptorTable;
	RefPtr<Volt::RHI::RenderPipeline> m_renderPipeline;
};
