#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class Shader;
	class ImageView;
	class BufferView;
	class BufferViewSet;

	struct DescriptorTableSpecification
	{
		Ref<Shader> shader;
	};

	class DescriptorTable : public RHIInterface
	{
	public:
		virtual void SetImageView(Ref<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetBufferView(Ref<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;
		virtual void SetBufferViewSet(Ref<BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) = 0;

		static Ref<DescriptorTable> Create(const DescriptorTableSpecification& specification);

	protected:
		DescriptorTable() = default;
	};
}
