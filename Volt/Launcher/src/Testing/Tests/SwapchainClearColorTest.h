#pragma once

#include "Testing/RenderingTestBase.h"

#include <RHIModule/Buffers/CommandBufferSet.h>

class SwapchainClearColorTest : public RenderingTestBase
{
public:
	SwapchainClearColorTest();
	~SwapchainClearColorTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "SwapchainClearColorTest"; }

private:
	Volt::RHI::CommandBufferSet m_commandBufferSet;
};
