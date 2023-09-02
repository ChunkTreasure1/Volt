#include "vtpch.h"
#include "TransientResourceSystem.h"

namespace Volt
{
	TransientResourceSystem::TransientResourceSystem()
	{
	}
	
	TransientResourceSystem::~TransientResourceSystem()
	{
	}

	Ref<RHI::Image2D> TransientResourceSystem::AquireImage2D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		return Ref<RHI::Image2D>();
	}

	Ref<RHI::StorageBuffer> TransientResourceSystem::AquireBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		return Ref<RHI::StorageBuffer>();
	}

	Ref<RHI::UniformBuffer> TransientResourceSystem::AquireUniformBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		return Ref<RHI::UniformBuffer>();
	}
}
