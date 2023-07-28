#pragma once
#include "VoltRHI/Graphics/GraphicsContext.h"

namespace Volt::RHI
{
	class MockGraphicsContext final : public GraphicsContext
	{
	public:
		MockGraphicsContext(const GraphicsContextCreateInfo& createInfo);
		~MockGraphicsContext() override = default;

	protected:
		void* GetHandleImpl() override;
	};
}
