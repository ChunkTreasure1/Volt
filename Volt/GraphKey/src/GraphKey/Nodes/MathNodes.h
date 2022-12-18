#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	class AddNode : public Node
	{
	public:
		inline AddNode()
		{
			name = "Add";

			inputs =
			{
				InputAttributeConfig<float>("A"),
				InputAttributeConfig<float>("B")
			};

			outputs =
			{
				OutputAttributeConfig<float>("Result", true, GK_BIND_FUNCTION(AddNode::Add))
			};
		}

	private:
		void Add()
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
			name = "Subtract";

			inputs =
			{
				InputAttributeConfig<float>("A"),
				InputAttributeConfig<float>("B")
			};

			outputs =
			{
				OutputAttributeConfig<float>("Result", true, GK_BIND_FUNCTION(SubtractNode::Subtract))
			};
		}

	private:
		void Subtract()
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
			name = "Multiply";

			inputs =
			{
				InputAttributeConfig<float>("A"),
				InputAttributeConfig<float>("B")
			};

			outputs =
			{
				OutputAttributeConfig<float>("Result", true, GK_BIND_FUNCTION(MultiplyNode::Multiply))
			};
		}

	private:
		void Multiply()
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
			name = "Division";

			inputs =
			{
				InputAttributeConfig<float>("A"),
				InputAttributeConfig<float>("B")
			};

			outputs =
			{
				OutputAttributeConfig<float>("Result", true, GK_BIND_FUNCTION(DivisionNode::Divide))
			};
		}

	private:
		void Divide()
		{
			const float val = GetInput<float>(0) / GetInput<float>(1);
			SetOutputData(0, val);
		}
	};
}