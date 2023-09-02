#include "vtpch.h"
#include "TransientResourceSystem.h"

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"

#include <VoltRHI/Core/RHIResource.h>
#include <VoltRHI/Images/Image2D.h>

namespace Volt
{
	TransientResourceSystem::TransientResourceSystem()
	{
	}
	
	TransientResourceSystem::~TransientResourceSystem()
	{
		m_allocatedResources.clear();
	}

	Ref<RHI::Image2D> TransientResourceSystem::AquireImage2D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle)->As<RHI::Image2D>();
		}

		RHI::ImageSpecification imageSpec{};
		imageSpec.width = imageDesc.width;
		imageSpec.height = imageDesc.height;
		imageSpec.depth = imageDesc.depth;
		imageSpec.layers = imageDesc.layers;
		imageSpec.mips = imageDesc.mips;

		imageSpec.format = imageDesc.format;
		imageSpec.usage = imageDesc.usage;
		imageSpec.debugName = imageDesc.name;
		imageSpec.isCubeMap = imageDesc.isCubeMap;

		Ref<RHI::Image2D> image = RHI::Image2D::Create(imageSpec);
		m_allocatedResources[resourceHandle] = image;

		return image;
	}

	Ref<RHI::StorageBuffer> TransientResourceSystem::AquireBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle)->As<RHI::StorageBuffer>();
		}

		return Ref<RHI::StorageBuffer>();
	}

	Ref<RHI::UniformBuffer> TransientResourceSystem::AquireUniformBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		return Ref<RHI::UniformBuffer>();
	}
}
