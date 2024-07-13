#pragma once

#include "Testing/RenderingTestBase.h"

class RG_DrawWithMeshShaderTest : public RenderingTestBase
{
public:
	RG_DrawWithMeshShaderTest();
	~RG_DrawWithMeshShaderTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_DrawWithMeshShaderTest"; }

private:
};
