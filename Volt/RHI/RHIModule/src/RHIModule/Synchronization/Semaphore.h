#pragma once

#include "RHIModule/Core/RHIInterface.h"

namespace Volt::RHI
{
	struct SemaphoreCreateInfo 
	{
		uint64_t initialValue = 0;
	};

	class VTRHI_API Semaphore : public RHIInterface
	{
	public:
		virtual ~Semaphore() = default;

		virtual void Wait() = 0;
		virtual void Signal(const uint64_t signalValue) = 0;
		
		virtual const uint64_t GetValue() const = 0;
		virtual const uint64_t IncrementAndGetValue() = 0;

		static RefPtr<Semaphore> Create(const SemaphoreCreateInfo& createInfo);

	protected:	
		Semaphore() = default;
	};
}
