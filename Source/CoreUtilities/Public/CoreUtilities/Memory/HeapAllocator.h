#pragma once

#include "CoreUtilities/Core.h"
#include "CoreUtilities/Profiling/Profiling.h"

class HeapAllocator
{
public:
	template<typename T, typename... Args>
	VT_INLINE static T* Allocate(Args&&... args)
	{
		VT_PROFILE_FUNCTION();

		s_totalAllocated += sizeof(T);

		T* ptr = new T(std::forward<Args>(args)...);

		VT_PROFILE_ALLOC(ptr, sizeof(T));
		return ptr;
	}

	template<typename T>
	VT_INLINE static void Free(T* ptr)
	{
		VT_PROFILE_FUNCTION();

		s_totalAllocated -= sizeof(T);
		
		VT_PROFILE_FREE(ptr);
		delete ptr;
	}

	VT_INLINE static void* AllocateUninitialized(size_t size, size_t alignment)
	{
		void* result;
		if (alignment <= MIN_PLATFORM_ALIGNMENT)
		{
			result = malloc(size);
		}
		else
		{
#ifdef VT_PLATFORM_WINDOWS
			result = _aligned_malloc(size, alignment);
#else
			result = malloc(size);
#endif
		}

		VT_PROFILE_ALLOC(result, size);
		return result;
	}

	VT_INLINE static void FreeUninitialized(void* ptr)
	{
		VT_PROFILE_FREE(ptr);
		delete[](char*)ptr;
	}

private:
	inline static std::atomic<uint64_t> s_totalAllocated = 0;
};
