#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	struct ParameterNode : public Node
	{
		inline ~ParameterNode() override = default;
		Volt::UUID parameterId;
	};

	template<typename T>
	struct GetParameterNode : public ParameterNode
	{
		inline GetParameterNode();
		~GetParameterNode() override = default;

		inline const std::string GetName() override { return "Get " + myGraph->GetParameterNameFromId(parameterId); }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

	private:
		void GetParameter();
	};

	template<typename T>
	struct SetParameterNode : public ParameterNode
	{
		inline SetParameterNode();
		~SetParameterNode() override = default;

		inline const std::string GetName() override { return "Set " + myGraph->GetParameterNameFromId(parameterId); }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

	private:
		void SetParameter();
	};

	template<typename T>
	inline GetParameterNode<T>::GetParameterNode()
	{
		outputs =
		{
			AttributeConfig<T>("Get", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetParameterNode::GetParameter))
		};
	}

	template<typename T>
	inline void GetParameterNode<T>::GetParameter()
	{
		if (!myGraph->HasParameter(parameterId))
		{
			SetOutputData(0, T{});
			return;
		}

		SetOutputData(0, myGraph->GetParameterValue<T>(parameterId));
	}
	template<typename T>
	inline SetParameterNode<T>::SetParameterNode()
	{
		inputs =
		{
			AttributeConfig("Set", AttributeDirection::Input, GK_BIND_FUNCTION(SetParameterNode::SetParameter)),
			AttributeConfig<T>("Value", AttributeDirection::Input)
		};

		outputs =
		{
			AttributeConfig("", AttributeDirection::Output),
			AttributeConfig<T>("Value", AttributeDirection::Output, true)
		};
	}

	template<typename T>
	inline void SetParameterNode<T>::SetParameter()
	{
		if (!myGraph->HasParameter(parameterId))
		{
			return;
		}

		myGraph->SetParameterValue(parameterId, GetInput<T>(1));
		SetOutputData(1, myGraph->GetParameterValue<T>(parameterId));
		ActivateOutput(0);
	}
}
