#pragma once

#include "Testing/RenderingTestBase.h"

class RG_ShaderRuntimeValidationTest : public RenderingTestBase
{
public:
	RG_ShaderRuntimeValidationTest();
	~RG_ShaderRuntimeValidationTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_ShaderRuntimeValidationTest"; }
};
