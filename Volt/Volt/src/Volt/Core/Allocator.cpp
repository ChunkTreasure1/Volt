#include "vtpch.h"
#include "Allocator.h"

#include "Volt/Core/Profiling.h"

inline static size_t s_totalAllocation = 0;

void* operator new(size_t size)
{
	s_totalAllocation += size;
	void* p = malloc(size);

	VT_PROFILE_ALLOC(p, size);

	return p;
}

void operator delete(void* p)
{
	VT_PROFILE_FREE(p);
	free(p);
}
