#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include "VoltRHI/Images/Image2D.h"

namespace Volt::RHI
{
	class Allocation;
	class MemoryPool;

	class Allocator : public RHIInterface
	{
	public:
		virtual ~Allocator() = default;

		virtual Ref<Allocation> CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage = MemoryUsage::GPU) = 0;
		virtual Ref<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage = MemoryUsage::GPU) = 0;

		virtual void DestroyBuffer(Ref<Allocation> allocation) = 0;
		virtual void DestroyImage(Ref<Allocation> allocation) = 0;

	protected:
		Allocator() = default;
	};

	class DefaultAllocator : public Allocator
	{
	public:
		virtual ~DefaultAllocator() override = default;

		static Scope<DefaultAllocator> Create();

	protected:
		DefaultAllocator() = default;
	};

	class TransientAllocator : public Allocator
	{
	public:
		virtual ~TransientAllocator() override = default;

		static Ref<TransientAllocator> Create();

	protected:
		TransientAllocator() = default;
	};
}