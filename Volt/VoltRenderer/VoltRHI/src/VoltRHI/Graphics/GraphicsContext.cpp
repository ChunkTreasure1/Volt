#include "rhipch.h"
#include "GraphicsContext.h"

namespace Volt
{
	Ref<GraphicsContext> GraphicsContext::Create(const GraphicsContextCreateInfo& createInfo)
	{
		s_graphicsAPI = createInfo.graphicsApi;
		return Ref<GraphicsContext>();
	}
}
