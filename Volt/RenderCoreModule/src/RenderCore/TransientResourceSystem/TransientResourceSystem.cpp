#include "rcpch.h"
#include "TransientResourceSystem.h"

#include "RenderCore/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphBufferResource.h"

#include <RHIModule/Core/RHIResource.h>
#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/UniformBuffer.h>
#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Images/Image.h>

namespace Volt
{
	// #TODO_Ivar: Readd
	//static ConsoleVariable<int32_t> s_enableMemoryAliasingCVar = ConsoleVariable<int32_t>("r.enableMemoryAliasing", 0, "Control whether memory aliasing should be enabled");

	TransientResourceSystem::TransientResourceSystem()
	{
	}
	
	TransientResourceSystem::~TransientResourceSystem()
	{
		m_allocatedResources.clear(); 
	}

	WeakPtr<RHI::Image> TransientResourceSystem::AquireImage(RenderGraphImageHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		return AquireImageRef(resourceHandle, imageDesc);
	}

	WeakPtr<RHI::StorageBuffer> TransientResourceSystem::AquireBuffer(RenderGraphBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		return AquireBufferRef(resourceHandle, bufferDesc);
	}

	WeakPtr<RHI::UniformBuffer> TransientResourceSystem::AquireUniformBuffer(RenderGraphUniformBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		return AquireUniformBufferRef(resourceHandle, bufferDesc);
	}

	RefPtr<RHI::Image> TransientResourceSystem::AquireImageRef(RenderGraphImageHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle).resource.As<RHI::Image>();
		}

		//const size_t hash = Utility::GetHashFromImageDesc(imageDesc);
		//if (/*s_enableMemoryAliasingCVar.GetValue()*/ false && m_surrenderedResources.contains(hash))
		//{
		//	if (!m_surrenderedResources.at(hash).empty())
		//	{
		//		RenderGraphResourceHandle surrenderedHandle = m_surrenderedResources.at(hash).back();
		//		m_surrenderedResources.at(hash).pop_back();

		//		RefPtr<RHI::RHIResource> resource = m_allocatedResources.at(surrenderedHandle).resource;
		//		m_allocatedResources[resourceHandle].resource = resource;

		//		return resource.As<RHI::Image2D>();
		//	}
		//}

		RHI::ImageSpecification imageSpec{};
		imageSpec.width = imageDesc.width;
		imageSpec.height = imageDesc.height;
		imageSpec.depth = imageDesc.depth;
		imageSpec.layers = imageDesc.layers;
		imageSpec.mips = imageDesc.mips;

		if (imageDesc.type == ResourceType::Image2D)
		{
			imageSpec.imageType = RHI::ResourceType::Image2D;
		} 
		else if (imageDesc.type == ResourceType::Image3D)
		{
			imageSpec.imageType = RHI::ResourceType::Image3D;
		}

		imageSpec.format = imageDesc.format;
		imageSpec.usage = imageDesc.usage;
		imageSpec.debugName = imageDesc.name;
		imageSpec.isCubeMap = imageDesc.isCubeMap;
		imageSpec.initializeImage = false;

		RefPtr<RHI::Image> image = RHI::Image::Create(imageSpec, nullptr, RHI::GraphicsContext::GetTransientAllocator());

		ResourceInfo info{};
		info.resource = image;
		info.isOriginal = true;

		m_allocatedResources[resourceHandle] = info;

		return image;
	}

	RefPtr<RHI::StorageBuffer> TransientResourceSystem::AquireBufferRef(RenderGraphBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle).resource.As<RHI::StorageBuffer>();
		}

		//const size_t hash = Utility::GetHashFromBufferDesc(bufferDesc);
		//if (/*s_enableMemoryAliasingCVar.GetValue()*/ false && m_surrenderedResources.contains(hash))
		//{
		//	if (!m_surrenderedResources.at(hash).empty())
		//	{
		//		RenderGraphResourceHandle surrenderedHandle = m_surrenderedResources.at(hash).back();
		//		m_surrenderedResources.at(hash).pop_back();

		//		RefPtr<RHI::RHIResource> resource = m_allocatedResources.at(surrenderedHandle).resource;
		//		m_allocatedResources[resourceHandle].resource = resource;

		//		return resource.As<RHI::StorageBuffer>();
		//	}
		//}

		// #TODO_Ivar: Switch to transient allocations
		RefPtr<RHI::StorageBuffer> buffer = RHI::StorageBuffer::Create(bufferDesc.count, bufferDesc.elementSize, bufferDesc.name, bufferDesc.usage, bufferDesc.memoryUsage);

		ResourceInfo info{};
		info.resource = buffer;
		info.isOriginal = true;

		m_allocatedResources[resourceHandle] = info;

		return buffer;
	}

	RefPtr<RHI::UniformBuffer> TransientResourceSystem::AquireUniformBufferRef(RenderGraphUniformBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle).resource;
		}

		RefPtr<RHI::UniformBuffer> buffer = RHI::UniformBuffer::Create(static_cast<uint32_t>(bufferDesc.elementSize), nullptr, bufferDesc.count, bufferDesc.name);

		ResourceInfo info{};
		info.resource = buffer;
		info.isOriginal = true;

		m_allocatedResources[resourceHandle] = info;

		return buffer;
	}

	void TransientResourceSystem::SurrenderResource(RenderGraphResourceHandle originalResource, size_t hash)
	{
		m_surrenderedResources[hash].emplace_back(originalResource);
	}

	void TransientResourceSystem::AddExternalResource(RenderGraphResourceHandle resourceHandle, RefPtr<RHI::RHIResource> resource)
	{
		ResourceInfo info{};
		info.resource = resource;
		info.isOriginal = true;

		m_allocatedResources[resourceHandle] = info;
	}

	const uint64_t TransientResourceSystem::GetTotalAllocatedSize() const
	{
		uint64_t result = 0;

		for (const auto& [handle, info] : m_allocatedResources)
		{
			if (info.isOriginal)
			{
				result += info.resource->GetByteSize();
			}
		}

		return result;
	}
}
