#pragma once

#define DIRECTINPUT_VERSION 0x0800
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#include <windows.h>
#include <wchar.h>


#ifdef AK_WIN
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

// Windows Header Files:
#include <windows.h>
#endif
