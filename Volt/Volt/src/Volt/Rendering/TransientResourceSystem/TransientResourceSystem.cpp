#include "vtpch.h"
#include "TransientResourceSystem.h"

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphBufferResource.h"

#include "Volt/Console/ConsoleVariableRegistry.h"

#include <VoltRHI/Core/RHIResource.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Buffers/UniformBuffer.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/Image3D.h>

namespace Volt
{
	static ConsoleVariable<int32_t> s_enableMemoryAliasingCVar = ConsoleVariable<int32_t>("r.enableMemoryAliasing", 0, "Control whether memory aliasing should be enabled");

	TransientResourceSystem::TransientResourceSystem()
	{
	}
	
	TransientResourceSystem::~TransientResourceSystem()
	{
		m_allocatedResources.clear(); 
	}

	WeakPtr<RHI::Image2D> TransientResourceSystem::AquireImage2D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		return AquireImage2DRef(resourceHandle, imageDesc);
	}

	WeakPtr<RHI::Image3D> TransientResourceSystem::AquireImage3D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		return AquireImage3DRef(resourceHandle, imageDesc);
	}

	WeakPtr<RHI::StorageBuffer> TransientResourceSystem::AquireBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		return AquireBufferRef(resourceHandle, bufferDesc);
	}

	WeakPtr<RHI::UniformBuffer> TransientResourceSystem::AquireUniformBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		return AquireUniformBufferRef(resourceHandle, bufferDesc);
	}

	RefPtr<RHI::Image2D> TransientResourceSystem::AquireImage2DRef(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle).resource.As<RHI::Image2D>();
		}

		const size_t hash = Utility::GetHashFromImageDesc(imageDesc);
		if (s_enableMemoryAliasingCVar.GetValue() && m_surrenderedResources.contains(hash))
		{
			if (!m_surrenderedResources.at(hash).empty())
			{
				RenderGraphResourceHandle surrenderedHandle = m_surrenderedResources.at(hash).back();
				m_surrenderedResources.at(hash).pop_back();

				RefPtr<RHI::RHIResource> resource = m_allocatedResources.at(surrenderedHandle).resource;
				m_allocatedResources[resourceHandle].resource = resource;

				return resource.As<RHI::Image2D>();
			}
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

		RefPtr<RHI::Image2D> image = RHI::Image2D::Create(imageSpec, nullptr, RHI::GraphicsContext::GetTransientAllocator());

		ResourceInfo info{};
		info.resource = image;
		info.isOriginal = true;

		m_allocatedResources[resourceHandle] = info;

		return image;
	}

	RefPtr<RHI::Image3D> TransientResourceSystem::AquireImage3DRef(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle).resource.As<RHI::Image3D>();
		}

		const size_t hash = Utility::GetHashFromImageDesc(imageDesc);
		if (s_enableMemoryAliasingCVar.GetValue() && m_surrenderedResources.contains(hash))
		{
			if (!m_surrenderedResources.at(hash).empty())
			{
				RenderGraphResourceHandle surrenderedHandle = m_surrenderedResources.at(hash).back();
				m_surrenderedResources.at(hash).pop_back();

				RefPtr<RHI::RHIResource> resource = m_allocatedResources.at(surrenderedHandle).resource;
				m_allocatedResources[resourceHandle].resource = resource;

				return resource.As<RHI::Image3D>();
			}
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
		imageSpec.imageType = RHI::ResourceType::Image3D;

		RefPtr<RHI::Image3D> image = RHI::Image3D::Create(imageSpec, nullptr, RHI::GraphicsContext::GetTransientAllocator());

		ResourceInfo info{};
		info.resource = image;
		info.isOriginal = true;

		m_allocatedResources[resourceHandle] = info;

		return image;
	}

	RefPtr<RHI::StorageBuffer> TransientResourceSystem::AquireBufferRef(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
	{
		VT_PROFILE_FUNCTION();

		if (m_allocatedResources.contains(resourceHandle))
		{
			return m_allocatedResources.at(resourceHandle).resource.As<RHI::StorageBuffer>();
		}

		const size_t hash = Utility::GetHashFromBufferDesc(bufferDesc);
		if (s_enableMemoryAliasingCVar.GetValue() && m_surrenderedResources.contains(hash))
		{
			if (!m_surrenderedResources.at(hash).empty())
			{
				RenderGraphResourceHandle surrenderedHandle = m_surrenderedResources.at(hash).back();
				m_surrenderedResources.at(hash).pop_back();

				RefPtr<RHI::RHIResource> resource = m_allocatedResources.at(surrenderedHandle).resource;
				m_allocatedResources[resourceHandle].resource = resource;

				return resource.As<RHI::StorageBuffer>();
			}
		}

		// #TODO_Ivar: Switch to transient allocations
		RefPtr<RHI::StorageBuffer> buffer = RHI::StorageBuffer::Create(bufferDesc.count, bufferDesc.elementSize, bufferDesc.name, bufferDesc.usage, bufferDesc.memoryUsage);

		ResourceInfo info{};
		info.resource = buffer;
		info.isOriginal = true;

		m_allocatedResources[resourceHandle] = info;

		return buffer;
	}

	RefPtr<RHI::UniformBuffer> TransientResourceSystem::AquireUniformBufferRef(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc)
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
