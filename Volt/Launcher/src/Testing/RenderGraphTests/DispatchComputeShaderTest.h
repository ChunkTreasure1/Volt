#pragma once

#include "Testing/RenderingTestBase.h"

class RG_DispatchComputeShaderTest : public RenderingTestBase
{
public:
	RG_DispatchComputeShaderTest();
	~RG_DispatchComputeShaderTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_DispatchComputeShaderTest"; }
};
