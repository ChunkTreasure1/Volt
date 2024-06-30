#pragma once

#include "CoreUtilities/Core.h"
#include "CoreUtilities/Profiling/Profiling.h"

class HeapAllocator
{
public:
	template<typename T, typename... Args>
	VT_INLINE static T* Allocate(Args&&... args)
	{
		s_totalAllocated += sizeof(T);

		T* ptr = new T(std::forward<Args>(args)...);

		VT_PROFILE_ALLOC(ptr, sizeof(T));
		return ptr;
	}

	template<typename T>
	VT_INLINE static void Free(T* ptr)
	{
		s_totalAllocated -= sizeof(T);
		
		VT_PROFILE_FREE(ptr);
		delete ptr;
	}

private:
	inline static uint64_t s_totalAllocated = 0;
};
