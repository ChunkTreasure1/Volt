#include "vtpch.h"
#include "Allocator.h"

inline static size_t s_totalAllocation = 0;

void* operator new(size_t size)
{
	s_totalAllocation += size;
	
	if (size == 0)
	{
		++size;
	}

	if (void* ptr = malloc(size))
	{
		//VT_PROFILE_ALLOC(ptr, size);
		return ptr;
	}

	throw std::bad_alloc();
}

void* operator new[](size_t size)
{
	s_totalAllocation += size;
	if (size == 0)
	{
		++size;
	}


	if (void* ptr = malloc(size))
	{
		//VT_PROFILE_ALLOC(ptr, size);
		return ptr;
	}

	throw std::bad_alloc();
}

void operator delete(void* p, size_t size) noexcept
{
	//VT_PROFILE_FREE(p);
	free(p);

	s_totalAllocation -= size;
}

void operator delete[](void* p, size_t size) noexcept
{
	//VT_PROFILE_FREE(p);
	free(p);

	s_totalAllocation -= size;
}

void Allocator::CheckAllocations()
{
	VT_CORE_ASSERT(s_totalAllocation == 0, "Memory leak!");
}
