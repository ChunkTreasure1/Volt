#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class Shader;
	class ImageView;
	class BufferView;
	class BufferViewSet;

	class SamplerState;

	struct DescriptorTableCreateInfo
	{
		Ref<Shader> shader;
		uint32_t count = 1;
	};

	class DescriptorTable : public RHIInterface
	{
	public:
		virtual void SetImageView(Ref<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetBufferView(Ref<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetBufferViewSet(Ref<BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;

		virtual void SetBufferViews(const std::vector<Ref<BufferView>>& bufferViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset = 0) = 0;
		virtual void SetImageViews(const std::vector<Ref<ImageView>>& imageViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset = 0) = 0;

		virtual void SetSamplerState(Ref<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;

		static Ref<DescriptorTable> Create(const DescriptorTableCreateInfo& specification);

	protected:
		DescriptorTable() = default;
	};
}