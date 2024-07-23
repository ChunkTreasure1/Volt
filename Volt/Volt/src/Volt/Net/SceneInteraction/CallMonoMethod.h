#pragma once

#include "Volt/Scene/Entity.h"

namespace Volt
{
	Vector<void*> GetMonoArguments(const Vector<uint8_t>& in_data);
	bool CallMonoMethod(const Entity& in_entity, const std::string& in_method, const Vector<uint8_t>& in_data);

}
