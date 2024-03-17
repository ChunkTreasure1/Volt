#pragma once

#include "VoltRHI/Core/RHIInterface.h"

#include <vector>

namespace Volt::RHI
{
	class Shader;
	class ImageView;
	class BufferView;
	class BufferViewSet;

	class SamplerState;

	class CommandBuffer;

	struct DescriptorTableCreateInfo
	{
		Ref<Shader> shader;
		uint32_t count = 1;
		bool isGlobal = false;
	};

	class DescriptorTable : public RHIInterface
	{
	public:
		// Set using bindings
		virtual void SetImageView(Ref<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetBufferView(Ref<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetBufferViewSet(Ref<BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;

		virtual void SetImageView(std::string_view name, Ref<ImageView> view, uint32_t arrayIndex) = 0;
		virtual void SetBufferView(std::string_view name, Ref<BufferView> view, uint32_t arrayIndex) = 0;
		virtual void SetSamplerState(std::string_view name, Ref<SamplerState> samplerState, uint32_t arrayIndex) = 0;

		virtual void SetBufferViews(const std::vector<Ref<BufferView>>& bufferViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset = 0) = 0;
		virtual void SetImageViews(const std::vector<Ref<ImageView>>& imageViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset = 0) = 0;

		virtual void SetSamplerState(Ref<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;

		virtual void Update(const uint32_t index = 0) = 0;

		static Ref<DescriptorTable> Create(const DescriptorTableCreateInfo& specification);

	protected:
		virtual void Bind(Ref<CommandBuffer> commandBuffer) = 0;

		DescriptorTable() = default;
	};
}
