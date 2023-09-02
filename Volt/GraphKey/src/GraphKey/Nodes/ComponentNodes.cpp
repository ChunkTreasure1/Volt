#include "gkpch.h"
#include "ComponentNodes.h"

#include "Volt/Scene/SceneManager.h"
#include "Volt/Events/ApplicationEvent.h"

namespace GraphKey
{
	void LightNode::Initialize()
	{
		inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().Get() };
	}

	void LightNode::SetProperties()
	{
		auto entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			if (entity.HasComponent<Volt::PointLightComponent>())
			{
				entity.GetComponent<Volt::PointLightComponent>().intensity = GetInput<float>(2);
				entity.GetComponent<Volt::PointLightComponent>().radius = GetInput<float>(3);
			}
		}

		ActivateOutput(0);
	}

	void ChangeLightColorNode::Initialize()
	{
		inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().Get()};
	}

	void ChangeLightColorNode::OnEvent(Volt::Event& e)
	{
		Volt::EventDispatcher dispather{e};
		dispather.Dispatch<Volt::AppUpdateEvent>([&](auto& e) 
		{
			if (!myHasStartedLerp)
			{
				return false;
			}

			if (myTargetEntity)
			{
				if (myTargetEntity.HasComponent<Volt::PointLightComponent>())
				{
					myTargetEntity.GetComponent<Volt::PointLightComponent>().color = glm::mix(myStartColor, myTargetColor, myLerpTime / myTotalLerpTime);
				}

				if (myTargetEntity.HasComponent<Volt::SpotLightComponent>())
				{
					myTargetEntity.GetComponent<Volt::SpotLightComponent>().color = glm::mix(myStartColor, myTargetColor, myLerpTime / myTotalLerpTime);
				}
			}

			myLerpTime += e.GetTimestep();

			if (myLerpTime > 1.f)
			{
				myHasStartedLerp = false;
			}

			return false;
		});
	}

	void ChangeLightColorNode::SetProperties()
	{
		auto entity = GetInput<Volt::Entity>(1);
		myTargetColor = GetInput<glm::vec3>(2);
		myTotalLerpTime = GetInput<float>(3);

		myTargetEntity = entity;
		myHasStartedLerp = true;

		if (entity)
		{
			if (entity.HasComponent<Volt::PointLightComponent>())
			{
				myStartColor = entity.GetComponent<Volt::PointLightComponent>().color;
			}

			if (entity.HasComponent<Volt::SpotLightComponent>())
			{
				myStartColor = entity.GetComponent<Volt::SpotLightComponent>().color;
			}
		}

		ActivateOutput(0);
	}
}
