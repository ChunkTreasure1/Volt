#pragma once

#include "Volt/Core/Base.h"

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	namespace RHI
	{
		class Image2D;
		class Image3D;
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

		WeakPtr<RHI::Image2D> AquireImage2D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		WeakPtr<RHI::Image3D> AquireImage3D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		WeakPtr<RHI::StorageBuffer> AquireBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);
		WeakPtr<RHI::UniformBuffer> AquireUniformBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);

		RefPtr<RHI::Image2D> AquireImage2DRef(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		RefPtr<RHI::Image3D> AquireImage3DRef(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		RefPtr<RHI::StorageBuffer> AquireBufferRef(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);
		RefPtr<RHI::UniformBuffer> AquireUniformBufferRef(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);

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
