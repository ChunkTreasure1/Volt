#pragma once

#include "Testing/RenderingTestBase.h"

class ClearBufferToValueTest : public RenderingTestBase
{
public:
	ClearBufferToValueTest();
	~ClearBufferToValueTest() override;

	bool RunTest();
	std::string GetName() const override { return "ClearBufferToValueTest"; }

private:
	RefPtr<Volt::RHI::StorageBuffer> m_buffer;
	RefPtr<Volt::RHI::StorageBuffer> m_readbackBuffer;
};
