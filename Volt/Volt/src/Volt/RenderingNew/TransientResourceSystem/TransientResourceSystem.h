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

		Ref<RHI::Image2D> AquireImage2D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		//Ref<RHI::Image3D> AquireImage3D(RenderGraphResourceHandle resourceHandle, const RenderGraphImageDesc& imageDesc);
		Ref<RHI::StorageBuffer> AquireBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);
		Ref<RHI::UniformBuffer> AquireUniformBuffer(RenderGraphResourceHandle resourceHandle, const RenderGraphBufferDesc& bufferDesc);
	private:

		std::unordered_map<RenderGraphResourceHandle, Ref<RHI::RHIResource>> m_allocatedResources;
	};
}
