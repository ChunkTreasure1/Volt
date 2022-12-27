#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	class AddNode : public Node
	{
	public:
		inline AddNode()
		{
			inputs =
			{
				AttributeConfig<float>("A", AttributeDirection::Input),
				AttributeConfig<float>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<float>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(AddNode::Add))
			};
		}

		inline const std::string GetName() override { return "Add"; }

		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Add()
		{
			const float val = GetInput<float>(0) + GetInput<float>(1);
			SetOutputData(0, val);
		}
	};

	class SubtractNode : public Node
	{
	public:
		inline SubtractNode()
		{
			inputs =
			{
				AttributeConfig<float>("A", AttributeDirection::Input),
				AttributeConfig<float>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<float>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(SubtractNode::Subtract))
			};
		}

		inline const std::string GetName() override { return "Subtract"; }

		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Subtract()
		{
			const float val = GetInput<float>(0) - GetInput<float>(1);
			SetOutputData(0, val);
		}
	};

	class MultiplyNode : public Node
	{
	public:
		inline MultiplyNode()
		{
			inputs =
			{
				AttributeConfig<float>("A", AttributeDirection::Input),
				AttributeConfig<float>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<float>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(MultiplyNode::Multiply))
			};
		}

		inline const std::string GetName() override { return "Multiply"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Multiply()
		{
			const float val = GetInput<float>(0) * GetInput<float>(1);
			SetOutputData(0, val);
		}
	};

	class DivisionNode : public Node
	{
	public:
		inline DivisionNode()
		{
			inputs =
			{
				AttributeConfig<float>("A", AttributeDirection::Input),
				AttributeConfig<float>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<float>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(DivisionNode::Divide))
			};
		}

		inline const std::string GetName() override { return "Division"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Divide()
		{
			const float val = GetInput<float>(0) / GetInput<float>(1);
			SetOutputData(0, val);
		}
	};
}