#pragma once

#include "Volt/Core/Base.h"

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	namespace RHI
	{
		class Image2D;
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

		Weak<RHI::Image2D> AquireImage2D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		//Ref<RHI::Image3D> AquireImage3D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		Weak<RHI::StorageBuffer> AquireBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);
		Weak<RHI::UniformBuffer> AquireUniformBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);

		void AddExternalResource(RenderGraphResourceHandle resourceHandle, Ref<RHI::RHIResource> resource);

	private:

		std::unordered_map<RenderGraphResourceHandle, Ref<RHI::RHIResource>> m_allocatedResources;
	};
}
