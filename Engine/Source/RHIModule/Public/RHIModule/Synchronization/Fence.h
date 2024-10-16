#pragma once

#include "RHIModule/Core/RHIInterface.h"

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

	class VTRHI_API Fence : public RHIInterface
	{
	public:
		static RefPtr<Fence> Create(const FenceCreateInfo& createInfo);

		virtual FenceStatus GetStatus() const = 0;
		virtual void Reset() const = 0;
		virtual void WaitUntilSignaled() const = 0;

	protected:
		Fence() = default;
	};
}
