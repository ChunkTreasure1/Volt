#include "cupch.h"
#include "Pointers/RefCounted.h"

#include "CoreUtilities/Memory/HeapAllocator.h"

RefCounted::RefCounted() noexcept
{
	m_weakCounter = HeapAllocator::Allocate<WeakCounter>();
}

void RefCounted::IncRef() const noexcept
{
	[[maybe_unused]] auto oldValue = m_count.fetch_add(1, std::memory_order_relaxed);
	assert(oldValue > 0);
}

void RefCounted::DecRef() const noexcept
{
	if (m_count.fetch_sub(1, std::memory_order_release) == 1)
	{
		m_weakCounter->m_isValid = false;
		m_weakCounter->Decrease();

		std::atomic_thread_fence(std::memory_order_acquire);
		HeapAllocator::Free(this);
	}
}

WeakCounter* RefCounted::GetWeakCounter() const
{
	return m_weakCounter;
}

void WeakCounter::Increase() const noexcept
{
	m_count.fetch_add(1, std::memory_order_relaxed);
}

void WeakCounter::Decrease() const noexcept
{
	if (m_count.fetch_sub(1, std::memory_order_release) == 1)
	{
		std::atomic_thread_fence(std::memory_order_acquire);
		HeapAllocator::Free(this);
	}
}

bool WeakCounter::IsValid() const noexcept
{
	return m_isValid;
}
