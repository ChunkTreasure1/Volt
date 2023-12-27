#include "vtpch.h"
#include "TransientResourceSystem.h"

#include "Volt/Core/Profiling.h"

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"

#include <VoltRHI/Core/RHIResource.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Buffers/UniformBuffer.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
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

	Weak<RHI::Image2D> TransientResourceSystem::AquireImage2D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		VT_PROFILE_FUNCTION();

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
		imageSpec.initializeImage = false;

		Ref<RHI::Image2D> image = RHI::Image2D::Create(imageSpec, RHI::GraphicsContext::GetTransientAllocator());
		m_allocatedResources[resourceHandle] = image;

		return image;
	}

	Weak<RHI::StorageBuffer> TransientResourceSystem::AquireBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle)->As<RHI::StorageBuffer>();
		}

		// #TODO_Ivar: Switch to transient allocations
		Ref<RHI::StorageBuffer> buffer = RHI::StorageBuffer::Create(bufferDesc.size, bufferDesc.name, bufferDesc.usage, bufferDesc.memoryUsage);
		m_allocatedResources[resourceHandle] = buffer;

		return buffer;
	}

	Weak<RHI::UniformBuffer> TransientResourceSystem::AquireUniformBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle)->As<RHI::UniformBuffer>();
		}

		Ref<RHI::UniformBuffer> buffer = RHI::UniformBuffer::Create(static_cast<uint32_t>(bufferDesc.size));
		m_allocatedResources[resourceHandle] = buffer;

		return buffer;
	}

	void TransientResourceSystem::AddExternalResource(RenderGraphResourceHandle resourceHandle, Ref<RHI::RHIResource> resource)
	{
		m_allocatedResources[resourceHandle] = resource;
	}

	const uint64_t TransientResourceSystem::GetTotalAllocatedSize() const
	{
		uint64_t result = 0;

		for (const auto& [handle, resource] : m_allocatedResources)
		{
			result += resource->GetByteSize();
		}

		return result;
	}
}
