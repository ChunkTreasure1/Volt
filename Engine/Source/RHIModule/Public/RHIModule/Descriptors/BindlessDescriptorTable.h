#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHICommon.h"
#include "RHIModule/Descriptors/ResourceHandle.h"

#include <CoreUtilities/Pointers/WeakPtr.h>

namespace Volt::RHI
{
	class StorageBuffer;
	class ImageView;
	class SamplerState;
	class CommandBuffer;
	class UniformBuffer;

	class VTRHI_API BindlessDescriptorTable : public RHIInterface
	{
	public:
		~BindlessDescriptorTable() override = default;

		virtual ResourceHandle RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer) = 0;
		virtual ResourceHandle RegisterImageView(WeakPtr<ImageView> imageView) = 0;
		virtual ResourceHandle RegisterSamplerState(WeakPtr<SamplerState> samplerState) = 0;

		virtual void UnregisterResource(ResourceHandle handle) = 0;
		virtual void MarkResourceAsDirty(ResourceHandle handle) = 0;

		virtual void UnregisterSamplerState(ResourceHandle handle) = 0;
		virtual void MarkSamplerStateAsDirty(ResourceHandle handle) = 0;

		virtual void Update() = 0;
		virtual void PrepareForRender() = 0;

		virtual void Bind(CommandBuffer& commandBuffer, WeakPtr<UniformBuffer> constantsBuffer, const uint32_t offsetIndex, const uint32_t stride) = 0;

		static RefPtr<BindlessDescriptorTable> Create(const uint64_t framesInFlight);

	protected:
		BindlessDescriptorTable() = default;
	};
}
