#pragma once

#include "CoreUtilities/CompilerTraits.h"

namespace Math
{
	VT_INLINE size_t HashCombine(const size_t lhs, const size_t rhs)
	{
		return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
	}
}
