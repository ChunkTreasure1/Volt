#pragma once

#include "Testing/RenderingTestBase.h"

class RG_DrawMeshShaderTriangleTest : public RenderingTestBase
{
public:
	RG_DrawMeshShaderTriangleTest();
	~RG_DrawMeshShaderTriangleTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_DrawMeshShaderTriangleTest"; }

private:
};
