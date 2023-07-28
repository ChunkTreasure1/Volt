#include "mkpch.h"
#include "MockGraphicsContext.h"

namespace Volt::RHI
{
	MockGraphicsContext::MockGraphicsContext(const GraphicsContextCreateInfo& createInfo)
	{
		createInfo;

		// WOW
	}

	void* MockGraphicsContext::GetHandleImpl()
	{
		return nullptr;
	}
}
