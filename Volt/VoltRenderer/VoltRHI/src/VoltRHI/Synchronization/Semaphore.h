#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	struct SemaphoreCreateInfo 
	{
		uint64_t initialValue = 0;
	};

	class Semaphore : public RHIInterface
	{
	public:
		virtual ~Semaphore() = default;

		virtual void Signal(const uint64_t signalValue) = 0;
		virtual void Wait(const uint64_t waitValue) = 0;
		virtual const uint64_t GetValue() const = 0;

		static Ref<Semaphore> Create(const SemaphoreCreateInfo& createInfo);

	protected:	
		Semaphore() = default;
	};
}
