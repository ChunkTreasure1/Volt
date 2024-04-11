#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include "VoltRHI/Images/Image2D.h"

namespace Volt::RHI
{
	class Allocation;
	class MemoryPool;

	class VTRHI_API Allocator : public RHIInterface
	{
	public:
		virtual ~Allocator() = default;

		virtual Ref<Allocation> CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage = MemoryUsage::GPU) = 0;
		virtual Ref<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage = MemoryUsage::GPU) = 0;

		virtual void DestroyBuffer(Ref<Allocation> allocation) = 0;
		virtual void DestroyImage(Ref<Allocation> allocation) = 0;

		// Used for allocation cache
		virtual void Update() = 0;

	protected:
		Allocator() = default;
	};

	class VTRHI_API DefaultAllocator : public Allocator
	{
	public:
		virtual ~DefaultAllocator() override = default;

		static Scope<DefaultAllocator> Create();

	protected:
		DefaultAllocator() = default;
	};

	class VTRHI_API TransientAllocator : public Allocator
	{
	public:
		virtual ~TransientAllocator() override = default;

		static Ref<TransientAllocator> Create();

	protected:
		TransientAllocator() = default;
	};
}
