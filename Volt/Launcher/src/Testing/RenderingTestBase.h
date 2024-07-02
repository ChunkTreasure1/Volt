#pragma once

#include <VoltRHI/Buffers/CommandBuffer.h>

class RenderingTestBase
{
public:
	RenderingTestBase();
	virtual ~RenderingTestBase();

	virtual void RunTest() = 0;

protected:
	RefPtr<Volt::RHI::CommandBuffer> m_commandBuffer;
};
