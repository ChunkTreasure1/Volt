#include "rcpch.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	bool RenderGraphResourceHandle::operator==(const RenderGraphNullHandle& other)
	{
		return m_value == other.Get();
	}
}
