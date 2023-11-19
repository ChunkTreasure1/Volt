#pragma once

#include <Volt/Core/UUID.h>

namespace GraphKey
{
	struct Link
	{
		UUID64 input = 0;
		UUID64 output = 0;

		UUID64 id{};
	};
}
