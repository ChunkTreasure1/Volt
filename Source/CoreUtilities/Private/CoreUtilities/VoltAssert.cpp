#include "cupch.h"
#include "VoltAssert.h"

#ifdef VT_PLATFORM_WINDOWS
	#if defined(_MSC_VER)
		#include <crtdbg.h>
	#endif

#include "CoreUtilities/Platform/Windows/VoltWindows.h"
#endif

void AssertionFailure(const char* expression)
{
#if defined(VT_ENABLE_ASSERTS) || defined(VT_ENABLE_ENSURES)
	#ifdef VT_PLATFORM_WINDOWS
	printf("%s\n", expression);
	if (::IsDebuggerPresent())
	{
		OutputDebugStringA(expression);
	}
	#else	
	printf("%s\n", expression);
	#endif
#elif VT_ENABLE_ENSURES
	VT_UNUSED(expression);
#endif

	VT_DEBUGBREAK();
}

void AssertionFailure(std::string_view expression)
{
	AssertionFailure(expression.data());
}
