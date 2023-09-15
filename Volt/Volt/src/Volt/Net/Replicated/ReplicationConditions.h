#pragma once

#include "Volt/Scene/Serialization/ComponentRegistry.h"
#include "Volt/Scene/Serialization/ComponentReflection.h"

namespace Volt
{
	enum class eRepCondition : uint32_t
	{
		OFF = 0,
		CONTINUOUS,
		NOTIFY,
	};

	static void ReflectType(TypeDesc<eRepCondition>& reflect)
	{
		reflect.SetGUID("{5A70DDBB-E05F-4854-B73B-4667E45997AB}"_guid);
		reflect.SetLabel("Replication Condition");
		reflect.SetDefaultValue(eRepCondition::OFF);
		reflect.AddConstant(eRepCondition::OFF, "off", "Off");
		reflect.AddConstant(eRepCondition::CONTINUOUS, "continuous", "Continuous");
		reflect.AddConstant(eRepCondition::NOTIFY, "notify", "Notify");
	}
}
