#include "cupch.h"

#ifdef VT_PLATFORM_WINDOWS

#include "CoreUtilities/Atomic.h"
#include "CoreUtilities/Platform/Windows/VoltWindows.h"

#undef InterlockedCompareExchange
#undef InterlockedExchange
#undef InterlockedIncrement
#undef InterlockedDecrement

namespace Atomic
{
	long InterlockedCompareExchange(long volatile* destination, long exchange, long comperand)
	{
		return ::_InterlockedCompareExchange(destination, exchange, comperand);
	}

	long InterlockedExchange(long volatile* destination, long value)
	{
		return ::_InterlockedExchange(destination, value);
	}

	long InterlockedIncrement(long volatile* destination)
	{
		return ::_InterlockedIncrement(destination);
	}

	long InterlockedDecrement(long volatile* destination)
	{
		return ::_InterlockedDecrement(destination);
	}
}

#endif
