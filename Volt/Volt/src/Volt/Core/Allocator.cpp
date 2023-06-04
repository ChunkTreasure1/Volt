#include "vtpch.h"
#include "Allocator.h"

inline static size_t s_totalAllocation = 0;

void* operator new(size_t size)
{
	s_totalAllocation += size;
	void* p = malloc(size);
	return p;
}

void operator delete(void* p)
{
	free(p);
}
