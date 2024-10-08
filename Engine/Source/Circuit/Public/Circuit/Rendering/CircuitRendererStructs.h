#pragma once
#include <RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h>

struct CircuitOutputData
{
	Volt::RenderGraphImageHandle outputTextureHandle;
	Volt::RenderGraphBufferHandle uiCommandsBufferHandle;
};
