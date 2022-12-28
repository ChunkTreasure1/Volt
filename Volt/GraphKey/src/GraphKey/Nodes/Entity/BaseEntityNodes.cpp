#include "gkpch.h"
#include "BaseEntityNodes.h"

#include <Volt/Scene/SceneManager.h>

namespace GraphKey
{
	CreateEntityNode::CreateEntityNode()
	{
		inputs =
		{
			AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(CreateEntityNode::CreateEntity)),
			AttributeConfig<std::string>("Tag", AttributeDirection::Input),
			AttributeConfig<gem::vec3>("Position", AttributeDirection::Input),
			AttributeConfig<gem::quat>("Rotation", AttributeDirection::Input),
			AttributeConfig<gem::vec3>("Scale", AttributeDirection::Input),
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output),
			AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Output)
		};
	}

	void CreateEntityNode::CreateEntity()
	{
		auto activeScene = Volt::SceneManager::GetActiveScene();
		if (activeScene)
		{
			auto entity = activeScene->CreateEntity();
			entity.SetPosition(GetInput<gem::vec3>(2));
			entity.SetLocalRotation(GetInput<gem::quat>(3));
			entity.SetLocalScale(GetInput<gem::vec3>(4));

			SetOutputData(1, entity);
		};

		ActivateOutput(0);
	}
	
	EntityNode::EntityNode()
	{
		outputs =
		{
			AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Output)
		};
	}
}