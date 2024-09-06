#pragma once

#include <Tracy.hpp>

#define VT_PROFILING_METHOD_NONE 0
#define VT_PROFILING_METHOD_TRACY 2

#define VT_PROFILING_METHOD VT_PROFILING_METHOD_TRACY

#if VT_PROFILING_METHOD == VT_PROFILING_METHOD_TRACY

#define VT_PROFILE_FRAME(NAME) { FrameMark; ZoneTransientN(__tracyScope, NAME, true); }
#define VT_PROFILE_FUNCTION(...)  ZoneTransient(___tracy_scoped_zone, true)
#define VT_PROFILE_TAG(NAME, ...)
#define VT_PROFILE_SCOPE(NAME) ZoneTransientN(__tracyScope, NAME, true)
#define VT_PROFILE_THREAD(...) tracy::SetThreadName(__VA_ARGS__)
#define VT_PROFILE_CATEGORY(...)

#define VT_PROFILE_ALLOC(ptr, size) TracyAlloc(ptr, size)
#define VT_PROFILE_FREE(ptr) TracyFree(ptr)

#else

#define VT_PROFILE_FRAME(...)
#define VT_PROFILE_FUNCTION(...)
#define VT_PROFILE_TAG(NAME, ...)
#define VT_PROFILE_SCOPE(NAME)
#define VT_PROFILE_THREAD(...)
#define VT_PROFILE_CATEGORY(...)

#define VT_PROFILE_ALLOC(ptr, size)
#define VT_PROFILE_FREE(ptr)

#endif
