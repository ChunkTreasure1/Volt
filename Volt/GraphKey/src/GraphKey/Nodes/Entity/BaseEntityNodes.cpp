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
			AttributeConfig<glm::vec3>("Position", AttributeDirection::Input),
			AttributeConfig<glm::quat>("Rotation", AttributeDirection::Input),
			AttributeConfig<glm::vec3>("Scale", AttributeDirection::Input),
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
		auto scenePtr = activeScene.lock();

		if (scenePtr)
		{
			auto entity = scenePtr->CreateEntity();
			entity.SetPosition(GetInput<glm::vec3>(2));
			entity.SetLocalRotation(GetInput<glm::quat>(3));
			entity.SetLocalScale(GetInput<glm::vec3>(4));

			SetOutputData(1, entity);
		};

		ActivateOutput(0);
	}

	EntityNode::EntityNode()
	{
		outputs =
		{
			AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Output, false, GK_BIND_FUNCTION(EntityNode::GetEntity))
		};
	}

	void EntityNode::GetEntity()
	{
		SetOutputData(0, std::any_cast<Volt::Entity>(outputs[0].data));
	}

	void EntityNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(outputs[0].data))
		{
			outputs[0].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
		}
	}

	DestroyEntityNode::DestroyEntityNode()
	{
		inputs =
		{
			AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(DestroyEntityNode::DestroyEntity)),
			AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Input),
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output),
		};
	}

	void DestroyEntityNode::DestroyEntity()
	{
		auto activeScene = Volt::SceneManager::GetActiveScene();
		auto entity = GetInput<Volt::Entity>(1);

		auto scenePtr = activeScene.lock();

		if (scenePtr && entity)
		{
			scenePtr->RemoveEntity(entity);
		}

		ActivateOutput(0);
	}

	GetChildCountNode::GetChildCountNode()
	{
		inputs =
		{
			AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Input),
		};

		outputs =
		{
			AttributeConfig<int32_t>("Count", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetChildCountNode::GetChildCount)),
		};
	}

	void GetChildCountNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(inputs[0].data))
		{
			inputs[0].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
		}
	}

	void GetChildCountNode::GetChildCount()
	{
		auto entity = GetInput<Volt::Entity>(0);
		if (!entity)
		{
			return;
		}

		SetOutputData(0, (int32_t)entity.GetChildren().size());
	}

	SelfNode::SelfNode()
	{
		outputs =
		{
			AttributeConfig<Volt::Entity>("Self", AttributeDirection::Output, true, GK_BIND_FUNCTION(SelfNode::GetEntity))
		};
	}

	void SelfNode::Initialize()
	{
		outputs[0].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
	}

	void SelfNode::GetEntity()
	{
		SetOutputData(0, std::any_cast<Volt::Entity>(outputs[0].data));
	}
}
