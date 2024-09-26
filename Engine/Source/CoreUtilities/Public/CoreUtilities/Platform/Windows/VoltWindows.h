#pragma once

#ifdef VT_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

// Make sure that Winsock.h isn't included by Windows.h
#ifndef _WINSOCKAPI_
	#define _WINSOCKAPI_
#endif

#if defined(_WINDOWS_)
	#error "Windows.h" has been included by another file!
#endif

#include <Windows.h>

// We don't want to log to VS when in Dist.
#ifdef VT_DIST
	#undef OutputDebugString
	#define OutputDebugString(...) (void)0
#endif

#endif
