#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Descriptors/ResourceHandle.h"

#include <CoreUtilities/Pointers/WeakPtr.h>

namespace Volt::RHI
{
	class StorageBuffer;
	class ImageView;
	class SamplerState;
	class CommandBuffer;

	class VTRHI_API BindlessDescriptorTable : public RHIInterface
	{
	public:
		virtual ~BindlessDescriptorTable() {}

		virtual ResourceHandle RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer) = 0;
		virtual ResourceHandle RegisterImageView(WeakPtr<ImageView> imageView) = 0;
		virtual ResourceHandle RegisterSamplerState(WeakPtr<SamplerState> samplerState) = 0;

		virtual void UnregisterBuffer(ResourceHandle handle) = 0;
		virtual void UnregisterImageView(ResourceHandle handle, ImageViewType viewType) = 0;
		virtual void UnregisterSamplerState(ResourceHandle handle) = 0;

		virtual void MarkBufferAsDirty(ResourceHandle handle) = 0;
		virtual void MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType) = 0;
		virtual void MarkSamplerStateAsDirty(ResourceHandle handle) = 0;

		virtual ResourceHandle GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer) = 0; // #TODO_Ivar: This feels weird...

		virtual void Update() = 0;
		virtual void PrepareForRender() = 0;

		virtual void Bind(CommandBuffer& commandBuffer) = 0;

		static RefPtr<BindlessDescriptorTable> Create();

	protected:
		BindlessDescriptorTable() = default;
	};
}
