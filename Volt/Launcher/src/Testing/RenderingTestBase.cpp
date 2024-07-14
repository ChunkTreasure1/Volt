#include "RenderingTestBase.h"

RenderingTestBase::RenderingTestBase()
{
	m_commandBuffer = Volt::RHI::CommandBuffer::Create();
}

RenderingTestBase::~RenderingTestBase()
{
	m_commandBuffer = nullptr;
}
