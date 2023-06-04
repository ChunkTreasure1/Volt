#pragma once

#include <Wire/Serialization.h>

namespace Volt
{
	SERIALIZE_ENUM((enum class eRepCondition : uint32_t
	{
		OFF = 0,
		CONTINUOUS,
		NOTIFY,
	}), eRepCondition);
}
