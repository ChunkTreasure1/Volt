#pragma once

#include <Volt/Core/UUID.h>

namespace GraphKey
{
	struct Link
	{
		Volt::UUID input = 0;
		Volt::UUID output = 0;

		Volt::UUID id{};
	};
}