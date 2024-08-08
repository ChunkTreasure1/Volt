#include "rcpch.h"
#include "RenderGraphResourceHandle.h"

namespace Volt
{
	bool RenderGraphResourceHandle::operator==(const RenderGraphNullHandle& other)
	{
		return m_value == 0;
	}
}
