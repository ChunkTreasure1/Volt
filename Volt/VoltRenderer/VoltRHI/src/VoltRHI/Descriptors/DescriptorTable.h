#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class Shader;
	class ImageView;

	struct DescriptorTableSpecification
	{
		Ref<Shader> shader;
	};

	class DescriptorTable : public RHIInterface
	{
	public:
		virtual void SetImageView(uint32_t set, uint32_t binding, Ref<ImageView> image) = 0;

		static Ref<DescriptorTable> Create(const DescriptorTableSpecification& specification);

	protected:
		DescriptorTable() = default;
	};
}
