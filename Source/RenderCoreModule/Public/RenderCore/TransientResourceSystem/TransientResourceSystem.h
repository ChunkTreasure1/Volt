#pragma once

#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"

#include <CoreUtilities/Pointers/WeakPtr.h>
#include <CoreUtilities/Containers/ThreadSafeMap.h>

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

		TransientResourceSystem(const TransientResourceSystem& other);
		TransientResourceSystem(TransientResourceSystem&& other);
		TransientResourceSystem& operator=(const TransientResourceSystem& other);
		TransientResourceSystem& operator=(TransientResourceSystem&& other);

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

		vt::map<RenderGraphResourceHandle, ResourceInfo> m_allocatedResources;
		mutable std::mutex m_allocatedResourcesMutex;

		vt::map<size_t, Vector<RenderGraphResourceHandle>> m_surrenderedResources;
		mutable std::mutex m_surrenderedResourcesMutex;
	};
}
