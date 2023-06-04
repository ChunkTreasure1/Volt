#pragma once

#include "Volt/Scene/Entity.h"

namespace Volt
{
	std::vector<void*> GetMonoArguments(const std::vector<uint8_t>& in_data);
	bool CallMonoMethod(const Entity& in_entity, const std::string& in_method, const std::vector<uint8_t>& in_data);

}
