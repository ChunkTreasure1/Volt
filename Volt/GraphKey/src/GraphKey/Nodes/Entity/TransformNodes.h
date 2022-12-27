#pragma once

#include "GraphKey/Node.h"

#include <Volt/Scene/Entity.h>

#include <GEM/gem.h>

namespace GraphKey
{
	class GetEntityTransformNode : public Node
	{
	public:
		inline GetEntityTransformNode()
		{
			inputs =
			{
				AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<gem::vec3>("Position", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetPosition)),
				AttributeConfig<gem::quat>("Rotation", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetRotation)),
				AttributeConfig<gem::vec3>("Scale", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetEntityTransformNode::GetScale)),
			};
		}
		
		inline const std::string GetName() override { return "Get Entity Transform"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }
	
	private:
		inline void GetPosition()
		{
			const Volt::Entity entity = GetInput<Volt::Entity>(0);
			if (entity)
			{
				SetOutputData(0, entity.GetPosition());
			}
		}

		inline void GetRotation()
		{
			const Volt::Entity entity = GetInput<Volt::Entity>(0);
			if (entity)
			{
				SetOutputData(1, entity.GetRotation());
			}
		}

		inline void GetScale()
		{
			const Volt::Entity entity = GetInput<Volt::Entity>(0);
			if (entity)
			{
				SetOutputData(2, entity.GetScale());
			}
		}
	};

	class SetEntityTransformNode : public Node
	{
	public:
		inline SetEntityTransformNode()
		{
			inputs =
			{
				AttributeConfig("Set", AttributeDirection::Input, GK_BIND_FUNCTION(SetEntityTransformNode::SetTransform)),
				AttributeConfig<Volt::Entity>("Entity", GraphKey::AttributeDirection::Input),
				AttributeConfig<gem::vec3>("Position", AttributeDirection::Input),
				AttributeConfig<gem::quat>("Rotation", AttributeDirection::Input),
				AttributeConfig<gem::vec3>("Scale", AttributeDirection::Input),
			};
		}

		inline const std::string GetName() override { return "Set Entity Transform"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:
		inline void SetTransform()
		{
			Volt::Entity entity = GetInput<Volt::Entity>(1);
			if (entity)
			{
				entity.SetPosition(GetInput<gem::vec3>(2));
				entity.SetLocalRotation(GetInput<gem::quat>(3)); // #TODO_Ivar: Should set global
				entity.SetLocalScale(GetInput<gem::vec3>(4));
			}
		}
	};
}