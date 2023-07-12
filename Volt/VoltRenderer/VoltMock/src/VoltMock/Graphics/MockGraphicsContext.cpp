#include "mkpch.h"
#include "MockGraphicsContext.h"

namespace Volt
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
