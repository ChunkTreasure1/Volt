#pragma once

#include <VoltRHI/Buffers/CommandBuffer.h>

class RenderingTestBase
{
public:
	RenderingTestBase();
	virtual ~RenderingTestBase();

	virtual bool RunTest() = 0;
	virtual std::string GetName() const = 0;

protected:
	RefPtr<Volt::RHI::CommandBuffer> m_commandBuffer;
};
