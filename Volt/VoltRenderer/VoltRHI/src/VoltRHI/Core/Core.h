#pragma once

#include <CoreUtilities/Core.h>

#include <memory>
#include <string>

#define VT_NODISCARD [[nodiscard]]
#define VT_INLINE __forceinline
#define VT_DELETE_COMMON_OPERATORS(X) X(const X&) = delete; X& operator=(const X&) = delete; X(X&&) = delete; X& operator=(X&&) = delete

#ifdef VTRHI_BUILD_DLL
#define VTRHI_API __declspec(dllexport)
#else
#define VTRHI_API __declspec(dllimport)
#endif

#ifdef VT_DEBUG

#ifdef VT_PLATFORM_WINDOWS
	#define VT_RHI_DEBUGBREAK() __debugbreak();
#else
	#error "Debug break not implemented!"
#endif

#define VT_ENSURE(x) if (!(x)) { VT_RHI_DEBUGBREAK(); }
#define VT_ENABLE_GPU_MARKERS
#define VT_ENABLE_DEBUG_ALLOCATIONS
#define VT_ENABLE_COMMAND_BUFFER_VALIDATION
#define VT_ENABLE_NV_AFTERMATH

#elif VT_RELEASE

#define VT_RHI_DEBUGBREAK()
#define VT_ENSURE(x) if (!(x)) { VT_RHI_DEBUGBREAK(); }

#define VT_ENABLE_GPU_MARKERS
#define VT_ENABLE_COMMAND_BUFFER_VALIDATION
#define VT_ENABLE_NV_AFTERMATH

#elif VT_DIST

#define VT_RHI_DEBUGBREAK()
#define VT_ENSURE(x)


#endif
