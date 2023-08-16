#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class RHIResource;

	struct BufferViewSpecification
	{
		Weak<RHIResource> bufferResource;
	};

	class BufferView : public RHIInterface
	{
	public:
		~BufferView() override = default;

		static Ref<BufferView> Create(const BufferViewSpecification& specification);

	protected:
		BufferView() = default;
	};
}
