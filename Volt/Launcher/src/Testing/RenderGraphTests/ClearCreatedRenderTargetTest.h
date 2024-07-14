#pragma once

#include "Testing/RenderingTestBase.h"

class RG_ClearCreatedRenderTargetTest : public RenderingTestBase
{
public:
	RG_ClearCreatedRenderTargetTest();
	~RG_ClearCreatedRenderTargetTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_ClearCreatedRenderTargetTest"; }
};
