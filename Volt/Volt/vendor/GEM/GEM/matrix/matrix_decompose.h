#pragma once
#include "../constants.h"
#include "../epsilon.h"

#include "../quaternion/quaternion.h"

namespace gem
{
	template<typename T = float>
	vec<3, T> scale(vec<3, T> const& v, T desiredLength)
	{
		return v * desiredLength / length(v);
	}


}
