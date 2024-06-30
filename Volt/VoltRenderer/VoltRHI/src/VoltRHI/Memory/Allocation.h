#pragma once

#include "VoltRHI/Core/RHIInterface.h"

#include <CoreUtilities/UUID.h>

namespace Volt::RHI
{
	inline static constexpr uint32_t MAX_PAGE_COUNT = 5;

	struct AllocationBlock
	{
		uint64_t size = 0;
		uint64_t offset = 0;
		uint32_t pageId = 0;
	};

	struct PageAllocation
	{
		void* handle = nullptr;
		uint64_t size = 0;
		uint64_t alignment = 0;

		uint64_t usedSize = 0;
		uint64_t tail = 0;

		std::vector<AllocationBlock> availableBlocks;

		VT_NODISCARD VT_INLINE const uint64_t GetRemainingSize() const
		{
			return size - std::min(usedSize, size);
		}

		VT_NODISCARD VT_INLINE const uint64_t GetRemainingTailSize() const
		{
			return size - tail;
		}
	};

	class VTRHI_API Allocation : public RHIInterface
	{
	public:
		virtual ~Allocation() = default;

		template<typename T>
		constexpr T GetResourceHandle() const;

		template<typename T>
		constexpr T* Map();

		virtual void Unmap() = 0;
		[[nodiscard]] virtual const UUID64 GetHeapID() const = 0;
		[[nodiscard]] virtual const uint64_t GetDeviceAddress() const = 0;
		[[nodiscard]] virtual const size_t GetHash() const = 0;
		[[nodiscard]] virtual const uint64_t GetSize() const = 0;

	protected:
		friend class Allocator;

		virtual void* GetResourceHandleInternal() const = 0;
		virtual void* MapInternal() = 0;

		Allocation() = default;
	};

	class VTRHI_API TransientAllocation : public Allocation
	{
	public:
		virtual ~TransientAllocation() override = default;

	protected:
		TransientAllocation() = default;
	};

	template<typename T>
	constexpr inline T Allocation::GetResourceHandle() const
	{
		return reinterpret_cast<T>(GetResourceHandleInternal());
	}

	template<typename T>
	constexpr inline T* Allocation::Map()
	{
		return reinterpret_cast<T*>(MapInternal());
	}
}
