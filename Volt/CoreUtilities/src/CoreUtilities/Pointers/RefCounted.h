#pragma once

#include "CoreUtilities/Core.h"

#include <atomic>

class VTCOREUTIL_API WeakCounter
{
public:
	void Increase() const noexcept;
	void Decrease() const noexcept;

	bool IsValid() const noexcept;

private:
	friend class RefCounted;

	mutable std::atomic<int32_t> m_count = 1;
	std::atomic_bool m_isValid = true;
};

class VTCOREUTIL_API RefCounted
{
public:
	RefCounted() noexcept;

	void IncRef() const noexcept;
	void DecRef() const noexcept;

	WeakCounter* GetWeakCounter() const;

protected:
	virtual ~RefCounted() noexcept = default;

private:
	friend class HeapAllocator;

	mutable std::atomic<int32_t> m_count = 1;
	mutable WeakCounter* m_weakCounter = nullptr;
};
