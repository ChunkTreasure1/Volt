#include "vtpch.h"
#include "Volt/Components/CoreComponents.h"

#include "Volt/Utility/Random.h"

namespace Volt
{
	void CommonComponent::OnStart(CommonComponent& component, entt::entity entity)
	{
		component.timeSinceCreation = 0.f;
		component.randomValue = Random::Float(0.f, 1.f);
	}
}
