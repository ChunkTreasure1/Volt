#pragma once

#include "Testing/RenderingTestBase.h"

namespace Volt
{
	class Mesh;
}

class RG_DrawMeshShaderMeshTest : public RenderingTestBase
{
public:
	RG_DrawMeshShaderMeshTest();
	~RG_DrawMeshShaderMeshTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_DrawMeshShaderTriangleTest"; }

private:
	Ref<Volt::Mesh> m_mesh;
};
