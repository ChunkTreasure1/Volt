#include "gkpch.h"
#include "TransformNodes.h"

#include <Volt/Scene/SceneManager.h>

namespace GraphKey
{
	GetEntityTransformNode::GetEntityTransformNode()
	{
		inputs =
		{
			AttributeConfigDefault<Volt::Entity>("Entity", AttributeDirection::Input, Volt::Entity{ 0, nullptr })
		};

		outputs =
		{
			AttributeConfig<gem::vec3>("Position", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetPosition)),
			AttributeConfig<gem::vec3>("Rotation", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetRotation)),
			AttributeConfig<gem::vec3>("Scale", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetScale)),
		};
	}

	void GetEntityTransformNode::Initialize()
	{
		if (!std::any_cast<Volt::Entity>(inputs[0].data))
		{
			inputs[0].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().lock().get() };
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
			SetOutputData(1, gem::degrees(gem::eulerAngles(entity.GetRotation())));
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
			AttributeConfigDefault<gem::vec3>("Position", AttributeDirection::Input, gem::vec3{ 0.f }),
			AttributeConfigDefault<gem::vec3>("Rotation", AttributeDirection::Input, gem::vec3{ 0.f }),
			AttributeConfigDefault<gem::vec3>("Scale", AttributeDirection::Input, gem::vec3{ 1.f }),
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
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().lock().get() };
		}
	}

	void SetEntityTransformNode::SetTransform()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			entity.SetPosition(GetInput<gem::vec3>(2));
			entity.SetRotation(gem::radians(GetInput<gem::vec3>(3)));
			entity.SetScale(GetInput<gem::vec3>(4));
		}

		ActivateOutput(0);
	}

	SetEntityPositionNode::SetEntityPositionNode()
	{
		inputs =
		{
			AttributeConfig("Set", AttributeDirection::Input, GK_BIND_FUNCTION(SetEntityPositionNode::SetPosition)),
			AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
			AttributeConfigDefault<gem::vec3>("Position", AttributeDirection::Input, gem::vec3{0.f}),
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
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().lock().get() };
		}
	}

	void SetEntityPositionNode::SetPosition()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			entity.SetPosition(GetInput<gem::vec3>(2));
		}

		ActivateOutput(0);
	}

	SetEntityRotationNode::SetEntityRotationNode()
	{
		inputs =
		{
			AttributeConfig("Set", AttributeDirection::Input, GK_BIND_FUNCTION(SetEntityRotationNode::SetRotation)),
			AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
			AttributeConfigDefault<gem::vec3>("Rotation", AttributeDirection::Input, gem::vec3{0.f}),
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
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().lock().get() };
		}
	}

	void SetEntityRotationNode::SetRotation()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			entity.SetRotation(gem::quat{ gem::radians(GetInput<gem::vec3>(2)) });
		}

		ActivateOutput(0);
	}

	AddEntityRotationNode::AddEntityRotationNode()
	{
		inputs =
		{
			AttributeConfig("Add", AttributeDirection::Input, GK_BIND_FUNCTION(AddEntityRotationNode::AddRotation)),
			AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
			AttributeConfigDefault<gem::vec3>("Rotation", AttributeDirection::Input, gem::vec3{0.f}),
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
			inputs[1].data = Volt::Entity{ myGraph->GetEntity(), Volt::SceneManager::GetActiveScene().lock().get() };
		}
	}

	void AddEntityRotationNode::AddRotation()
	{
		Volt::Entity entity = GetInput<Volt::Entity>(1);
		if (entity)
		{
			gem::quat addRot = { gem::radians(GetInput<gem::vec3>(2)) };
			entity.SetRotation(entity.GetRotation() * addRot);
		}

		ActivateOutput(0);
	}
}