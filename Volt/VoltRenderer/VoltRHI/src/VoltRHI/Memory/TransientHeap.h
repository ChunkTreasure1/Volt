#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Core/RHIInterface.h"

#include <CoreUtilities/UUID.h>

namespace Volt::RHI
{
	class Allocation;

	enum class TransientHeapFlags
	{
		None = 0,
		AllowBuffers = BIT(0),
		AllowTextures = BIT(1),
		AllowRenderTargets = BIT(2),

		AllowAll = AllowBuffers | AllowTextures | AllowRenderTargets
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(TransientHeapFlags);

	struct TransientBufferCreateInfo
	{
		uint64_t size;
		BufferUsage usage;
		MemoryUsage memoryUsage;
	};

	struct TransientHeapCreateInfo
	{
		uint64_t pageSize = 0;
		uint32_t alignment = 0;
		TransientHeapFlags flags = TransientHeapFlags::AllowAll;
	};

	struct TransientImageCreateInfo
	{
		ImageSpecification imageSpecification;
		uint64_t size = 0;
	};

	class TransientHeap : public RHIInterface
	{
	public:
		virtual Ref<Allocation> CreateBuffer(const TransientBufferCreateInfo& createInfo) = 0;
		virtual Ref<Allocation> CreateImage(const TransientImageCreateInfo& createInfo) = 0;

		virtual void ForfeitBuffer(Ref<Allocation> allocation) = 0;
		virtual void ForfeitImage(Ref<Allocation> allocation) = 0;

		virtual const bool IsAllocationSupported(const uint64_t size, TransientHeapFlags heapFlags) const = 0;

		inline const UUID64 GetHeapID() const { return m_heapId; }

		static Ref<TransientHeap> Create(const TransientHeapCreateInfo& createInfo);

	protected:
		UUID64 m_heapId{};
	};
}