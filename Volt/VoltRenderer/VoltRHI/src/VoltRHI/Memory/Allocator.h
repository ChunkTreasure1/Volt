#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	class Allocation;
	class MemoryPool;

	class VTRHI_API Allocator : public RHIInterface
	{
	public:
		virtual ~Allocator() = default;

		virtual RefPtr<Allocation> CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage = MemoryUsage::GPU) = 0;
		virtual RefPtr<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage = MemoryUsage::GPU) = 0;

		virtual void DestroyBuffer(RefPtr<Allocation> allocation) = 0;
		virtual void DestroyImage(RefPtr<Allocation> allocation) = 0;

		// Used for allocation cache
		virtual void Update() = 0;

	protected:
		Allocator() = default;
	};

	class VTRHI_API DefaultAllocator : public Allocator
	{
	public:
		virtual ~DefaultAllocator() override = default;

		static RefPtr<DefaultAllocator> Create();

	protected:
		DefaultAllocator() = default;
	};

	class VTRHI_API TransientAllocator : public Allocator
	{
	public:
		virtual ~TransientAllocator() override = default;

		static RefPtr<TransientAllocator> Create();

	protected:
		TransientAllocator() = default;
	};
}
