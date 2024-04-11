#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class RHIResource;

	struct BufferViewSpecification
	{
		//Weak<RHIResource> bufferResource;
		RHIResource* bufferResource = nullptr;
	};

	class VTRHI_API BufferView : public RHIInterface
	{
	public:
		~BufferView() override = default;

		static Ref<BufferView> Create(const BufferViewSpecification& specification);
		[[nodiscard]] virtual const uint64_t GetDeviceAddress() const = 0;

	protected:
		BufferView() = default;
	};
}
