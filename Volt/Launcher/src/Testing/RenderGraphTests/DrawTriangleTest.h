#pragma once

#include "Testing/RenderingTestBase.h"

class RG_DrawTriangleTest : public RenderingTestBase
{
public:
	RG_DrawTriangleTest();
	~RG_DrawTriangleTest();

	bool RunTest() override;
	std::string GetName() const override { return "RG_DrawTriangleTest"; }

private:
};
