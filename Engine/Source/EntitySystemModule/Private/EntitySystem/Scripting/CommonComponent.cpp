#include "espch.h"

#include "EntitySystem/Scripting/CommonComponent.h"

namespace Volt
{
	void CommonComponent::OnStart(CommonEntity entity)
	{
		auto& component = entity.GetComponent<CommonComponent>();

		component.timeSinceCreation = 0.f;
		//component.randomValue = Random::Float(0.f, 1.f);
	}
}
