#pragma once

// Static analasys assume
#if defined (_MSC_VER) && (_MSC_VER >= 1300)
	#define VT_ANALASYS_ASSUME(x) __analysis_assume(!!(x))
#else
	#define VT_ANALASYS_ASSUME(x)
#endif

// Inline
#ifdef VT_PLATFORM_WINDOWS
	#define VT_INLINE __forceinline
#else
	#define VT_INLINE inline
#endif

// No discard
#define VT_NODISCARD [[nodiscard]]

// Optimize
#ifdef VT_ENABLE_OPTIMIZATION_TOGGLE
	#ifdef VT_PLATFORM_WINDOWS
		#define VT_OPTIMIZE_OFF __pragma(optimize("", off));
		#define VT_OPTIMIZE_ON __pragma(optimize("", on));
	#else
		#define VT_OPTIMIZE_OFF __pragma(optimize("", off));
		#define VT_OPTIMIZE_ON __pragma(optimize("", on));
	#endif
#else
	#define VT_OPTIMIZE_ON
	#define VT_OPTIMIZE_OFF
#endif

// Min alignment
#ifdef VT_PLATFORM_WINDOWS
	#define MIN_PLATFORM_ALIGNMENT 16
#else
	#error "Not defined!"
#endif

// Unused
template<typename T>
inline void VTBaseUnused(const volatile T& x) { (void)x; }
#define VT_UNUSED(x) VTBaseUnused(x)

// Debugbreak
#ifdef VT_PLATFORM_WINDOWS
	#define VT_DEBUGBREAK() __debugbreak()
#else
	#error "Not defined!"
#endif
