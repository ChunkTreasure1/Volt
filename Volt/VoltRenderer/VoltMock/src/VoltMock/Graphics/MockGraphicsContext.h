#pragma once
#include "VoltRHI/Graphics/GraphicsContext.h"
namespace Volt
{
	class MockGraphicsContext final : public GraphicsContext
	{
	public:
		MockGraphicsContext(const GraphicsContextCreateInfo& createInfo);
		~MockGraphicsContext() override = default;

	private:
	};
}
