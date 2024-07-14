#pragma once

#include "Testing/RenderingTestBase.h"

namespace Volt
{
	class Mesh;
}

class RG_DrawMeshShaderMultipleMeshesTest : public RenderingTestBase
{
public:
	RG_DrawMeshShaderMultipleMeshesTest();
	~RG_DrawMeshShaderMultipleMeshesTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_DrawMeshShaderMultipleMeshesTest"; }

private:
	Ref<Volt::Mesh> m_cubeMesh;
	Ref<Volt::Mesh> m_sphereMesh;
};
