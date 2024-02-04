#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	enum class FenceStatus : uint8_t
	{
		Signaled,
		Unsignaled,
		Error
	};

	struct FenceCreateInfo
	{
		bool createSignaled = false;
	};

	class Fence : public RHIInterface
	{
	public:
		static Ref<Fence> Create(const FenceCreateInfo& createInfo);

		virtual FenceStatus GetStatus() const = 0;
		virtual void Reset() const = 0;
		virtual void WaitUntilSignaled() const = 0;

	protected:
		Fence() = default;
	};
}
