#include "rhipch.h"

#include "RHIModule/Memory/AllocationCache.h"
#include "RHIModule/Memory/Allocation.h"

namespace Volt::RHI
{
	RefPtr<Allocation> AllocationCache::TryGetImageAllocationFromHash(const size_t hash)
	{
		std::scoped_lock lock{ m_imageAllocationMutex };
		for (int32_t i = static_cast<int32_t>(m_imageAllocations.size()) - 1; i >= 0; --i)
		{
			const auto alloc = m_imageAllocations.at(i);

			if (alloc.allocation->GetHash() == hash)
			{
				m_imageAllocations.erase(m_imageAllocations.begin() + i);
				return alloc.allocation;
			}
		}

		return nullptr;
	}

	RefPtr<Allocation> AllocationCache::TryGetBufferAllocationFromHash(const size_t hash)
	{
		std::scoped_lock lock{ m_bufferAllocationMutex };
		for (int32_t i = static_cast<int32_t>(m_bufferAllocations.size()) - 1; i >= 0; --i)
		{
			const auto alloc = m_bufferAllocations.at(i);

			if (alloc.allocation->GetHash() == hash)
			{
				m_bufferAllocations.erase(m_bufferAllocations.begin() + i);
				return alloc.allocation;
			}
		}

		return nullptr;
	}

	void AllocationCache::QueueImageAllocationForRemoval(RefPtr<Allocation> alloc)
	{
		std::scoped_lock lock{ m_imageAllocationMutex };
		m_imageAllocations.emplace_back(alloc, 0);
	}

	void AllocationCache::QueueBufferAllocationForRemoval(RefPtr<Allocation> alloc)
	{
		std::scoped_lock lock{ m_bufferAllocationMutex };
		m_bufferAllocations.emplace_back(alloc, 0);
	}

	AllocationsToRemove AllocationCache::UpdateAndGetAllocationsToDestroy()
	{
		for (auto& allocInfo : m_imageAllocations)
		{
			allocInfo.framesAlive++;
		}

		for (auto& allocInfo : m_bufferAllocations)
		{
			allocInfo.framesAlive++;
		}

		constexpr size_t MAX_FRAMES_ALIVE = 3;
		
		AllocationsToRemove result{};

		{
			std::scoped_lock lock{ m_imageAllocationMutex };
			for (int32_t i = static_cast<int32_t>(m_imageAllocations.size()) - 1; i >= 0; --i)
			{
				const auto& alloc = m_imageAllocations.at(i);
				if (alloc.framesAlive >= MAX_FRAMES_ALIVE)
				{
					result.imageAllocations.push_back(alloc.allocation);
					m_imageAllocations.erase(m_imageAllocations.begin() + i);
				}
			}
		}

		{
			std::scoped_lock lock{ m_bufferAllocationMutex };
			for (int32_t i = static_cast<int32_t>(m_bufferAllocations.size()) - 1; i >= 0; --i)
			{
				const auto& alloc = m_bufferAllocations.at(i);
				if (alloc.framesAlive >= MAX_FRAMES_ALIVE)
				{
					result.bufferAllocations.push_back(alloc.allocation);
					m_bufferAllocations.erase(m_bufferAllocations.begin() + i);
				}
			}
		}

		return result;
	}
}
