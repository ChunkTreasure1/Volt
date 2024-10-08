#pragma once

#include "CoreUtilities/Config.h"

#include <cstdint>

namespace Atomic
{
	extern VTCOREUTIL_API long InterlockedCompareExchange(long volatile* destination, long exchange, long comperand);
	extern VTCOREUTIL_API long InterlockedExchange(long volatile* destination, long value);
	extern VTCOREUTIL_API long InterlockedIncrement(long volatile* destination);
	extern VTCOREUTIL_API long InterlockedDecrement(long volatile* destination);
}
