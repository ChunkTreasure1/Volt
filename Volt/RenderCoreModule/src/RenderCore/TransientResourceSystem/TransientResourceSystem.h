#pragma once

#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"

#include <CoreUtilities/Pointers/WeakPtr.h>

namespace Volt
{
	namespace RHI
	{
		class Image;
		class StorageBuffer;
		class UniformBuffer;
		class RHIResource;
	}

	struct RenderGraphImageDesc;
	struct RenderGraphBufferDesc;

	class TransientResourceSystem
	{
	public:
		TransientResourceSystem();
		~TransientResourceSystem();

		WeakPtr<RHI::Image> AquireImage(RenderGraphImageHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		WeakPtr<RHI::StorageBuffer> AquireBuffer(RenderGraphBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);
		WeakPtr<RHI::UniformBuffer> AquireUniformBuffer(RenderGraphUniformBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);

		RefPtr<RHI::Image> AquireImageRef(RenderGraphImageHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		RefPtr<RHI::StorageBuffer> AquireBufferRef(RenderGraphBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);
		RefPtr<RHI::UniformBuffer> AquireUniformBufferRef(RenderGraphUniformBufferHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);

		void SurrenderResource(RenderGraphResourceHandle originalResource, size_t hash);
		void AddExternalResource(RenderGraphResourceHandle resourceHandle, RefPtr<RHI::RHIResource> resource);

		const uint64_t GetTotalAllocatedSize() const;

	private:
		struct ResourceInfo
		{
			RefPtr<RHI::RHIResource> resource;
			bool isOriginal = false;
		};

		std::unordered_map<RenderGraphResourceHandle, ResourceInfo> m_allocatedResources;
		std::unordered_map<size_t, Vector<RenderGraphResourceHandle>> m_surrenderedResources;
	};
}
