#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include <CoreUtilities/Pointers/WeakPtr.h>

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
		RefPtr<Shader> shader;
		bool isGlobal = false;
	};

	class VTRHI_API DescriptorTable : public RHIInterface
	{
	public:
		virtual void SetImageView(WeakPtr<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetBufferView(WeakPtr<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetSamplerState(WeakPtr<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;

		virtual void SetImageView(std::string_view name, WeakPtr<ImageView> view, uint32_t arrayIndex) = 0;
		virtual void SetBufferView(std::string_view name, WeakPtr<BufferView> view, uint32_t arrayIndex) = 0;
		virtual void SetSamplerState(std::string_view name, WeakPtr<SamplerState> samplerState, uint32_t arrayIndex) = 0;

		virtual void PrepareForRender() = 0;

		static RefPtr<DescriptorTable> Create(const DescriptorTableCreateInfo& specification);

	protected:
		virtual void Bind(CommandBuffer& commandBuffer) = 0;

		DescriptorTable() = default;
	};
}
