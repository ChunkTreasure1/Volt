#pragma once

#include "Testing/RenderingTestBase.h"

class SwapchainClearColorTest : public RenderingTestBase
{
public:
	SwapchainClearColorTest();
	~SwapchainClearColorTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "SwapchainClearColorTest"; }

private:
};
