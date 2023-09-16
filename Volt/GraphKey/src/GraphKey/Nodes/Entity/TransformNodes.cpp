#include "gkpch.h"
#include "TransformNodes.h"

#include <Volt/Scene/SceneManager.h>

namespace GraphKey
{
	GetEntityTransformNode::GetEntityTransformNode()
	{
		inputs =
		{
			AttributeConfigDefault<Volt::Entity>("Entity", AttributeDirection::Input, Volt::Entity::Null())
		};

		outputs =
		{
			AttributeConfig<glm::vec3>("Position", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetPosition)),
			AttributeConfig<glm::vec3>("Rotation", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetRotation)),
			AttributeConfig<glm::vec3>("Scale", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetScale)),
		};
	}

	void GetEntityTransformNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(inputs[0].data))
		{
			inputs[0].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
		}
	}

	void GetEntityTransformNode::GetPosition()
	{
		const Volt::Entity entity = GetInput<Volt::Entity>(0);
		if (entity)
		{
			SetOutputData(0, entity.GetPosition());
		}
	}

	void GetEntityTransformNode::GetRotation()
	{
		const Volt::Entity entity = GetInput<Volt::Entity>(0);
		if (entity)
		{
			SetOutputData(1, glm::degrees(glm::eulerAngles(entity.GetRotation())));
		}
	}

	void GetEntityTransformNode::GetScale()
	{
		const Volt::Entity entity = GetInput<Volt::Entity>(0);
		if (entity)
		{
			SetOutputData(2, entity.GetScale());
		}
	}

	SetEntityTransformNode::SetEntityTransformNode()
	{
		inputs =
		{
			AttributeConfig("Set", AttributeDirection::Input, GK_BIND_FUNCTION(SetEntityTransformNode::SetTransform)),
			AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
			AttributeConfigDefault<glm::vec3>("Position", AttributeDirection::Input, glm::vec3{ 0.f }),
			AttributeConfigDefault<glm::vec3>("Rotation", AttributeDirection::Input, glm::vec3{ 0.f }),
			AttributeConfigDefault<glm::vec3>("Scale", AttributeDirection::Input, glm::vec3{ 1.f }),
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output)
		};
	}

	void SetEntityTransformNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(inputs[1].data))
		{
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
		}
	}

	void SetEntityTransformNode::SetTransform()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			entity.SetPosition(GetInput<glm::vec3>(2));
			entity.SetRotation(glm::radians(GetInput<glm::vec3>(3)));
			entity.SetScale(GetInput<glm::vec3>(4));
		}

		ActivateOutput(0);
	}

	SetEntityPositionNode::SetEntityPositionNode()
	{
		inputs =
		{
			AttributeConfig("Set", AttributeDirection::Input, GK_BIND_FUNCTION(SetEntityPositionNode::SetPosition)),
			AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
			AttributeConfigDefault<glm::vec3>("Position", AttributeDirection::Input, glm::vec3{0.f}),
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output)
		};
	}

	void SetEntityPositionNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(inputs[1].data))
		{
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
		}
	}

	void SetEntityPositionNode::SetPosition()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			entity.SetPosition(GetInput<glm::vec3>(2));
		}

		ActivateOutput(0);
	}

	SetEntityRotationNode::SetEntityRotationNode()
	{
		inputs =
		{
			AttributeConfig("Set", AttributeDirection::Input, GK_BIND_FUNCTION(SetEntityRotationNode::SetRotation)),
			AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
			AttributeConfigDefault<glm::vec3>("Rotation", AttributeDirection::Input, glm::vec3{0.f}),
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output)
		};
	}

	void SetEntityRotationNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(inputs[1].data))
		{
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
		}
	}

	void SetEntityRotationNode::SetRotation()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			entity.SetRotation(glm::quat{ glm::radians(GetInput<glm::vec3>(2)) });
		}

		ActivateOutput(0);
	}

	AddEntityRotationNode::AddEntityRotationNode()
	{
		inputs =
		{
			AttributeConfig("Add", AttributeDirection::Input, GK_BIND_FUNCTION(AddEntityRotationNode::AddRotation)),
			AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
			AttributeConfigDefault<glm::vec3>("Rotation", AttributeDirection::Input, glm::vec3{0.f}),
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output)
		};
	}

	void AddEntityRotationNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(inputs[1].data))
		{
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene() };
		}
	}

	void AddEntityRotationNode::AddRotation()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			glm::quat addRot = { glm::radians(GetInput<glm::vec3>(2)) };
			entity.SetRotation(entity.GetRotation() * addRot);
		}

		ActivateOutput(0);
	}
}
