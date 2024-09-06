#pragma once

#include "Testing/RenderingTestBase.h"

#include <RHIModule/Buffers/CommandBufferSet.h>

namespace Volt
{
	class Mesh;
}

class RG_SimpleComputeShaderTest : public RenderingTestBase
{
public:
	RG_SimpleComputeShaderTest();
	~RG_SimpleComputeShaderTest() override;

	bool RunTest() override;
	std::string GetName() const override { return "RG_SimpleComputeShaderTest"; }

private:
	Ref<Volt::Mesh> m_cubeMesh;

	Volt::RHI::CommandBufferSet m_commandBufferSet;
};
