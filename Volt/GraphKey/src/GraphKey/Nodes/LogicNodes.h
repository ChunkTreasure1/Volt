#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	class BranchNode : public Node
	{
	public:
		BranchNode();
		~BranchNode() override = default;

		inline const std::string GetName() override { return "Branch"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		void Branch();
	};

	class DoOnceNode : public Node
	{
	public:
		DoOnceNode();
		~DoOnceNode() override = default;

		inline const std::string GetName() override { return "Do Once"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		bool myHasDone = false;

		void Do();
	};

	template<typename T>
	class EqualNode : public Node
	{
	public:
		inline EqualNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(EqualNode<T>::Equal))
			};
		}

		inline const std::string GetName() override { return "=="; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void Equal()
		{
			const auto lhs = GetInput<T>(0);
			const auto rhs = GetInput<T>(1);

			SetOutputData(0, lhs == rhs);
		}
	};

	template<typename T>
	class LessThanNode : public Node
	{
	public:
		inline LessThanNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(LessThanNode<T>::Less))
			};
		}

		inline const std::string GetName() override { return "<"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void Less()
		{
			const auto lhs = GetInput<T>(0);
			const auto rhs = GetInput<T>(1);

			SetOutputData(0, lhs < rhs);
		}
	};

	template<typename T>
	class GreaterThanNode : public Node
	{
	public:
		inline GreaterThanNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(GreaterThanNode<T>::Greater))
			};
		}

		inline const std::string GetName() override { return ">"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void Greater()
		{
			const auto lhs = GetInput<T>(0);
			const auto rhs = GetInput<T>(1);

			SetOutputData(0, lhs > rhs);
		}
	};

	template<typename T>
	class LessOrEqualNode : public Node
	{
	public:
		inline LessOrEqualNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(LessOrEqualNode<T>::LessOrEqual))
			};
		}

		inline const std::string GetName() override { return "<="; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void LessOrEqual()
		{
			const auto lhs = GetInput<T>(0);
			const auto rhs = GetInput<T>(1);

			SetOutputData(0, lhs <= rhs);
		}
	};

	template<typename T>
	class GreaterOrEqualNode : public Node
	{
	public:
		inline GreaterOrEqualNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(GreaterOrEqualNode<T>::GreaterOrEqual))
			};
		}

		inline const std::string GetName() override { return ">="; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void GreaterOrEqual()
		{
			const auto lhs = GetInput<T>(0);
			const auto rhs = GetInput<T>(1);

			SetOutputData(0, lhs >= rhs);
		}
	};

	class ForRangedNode : public Node
	{
	public:
		inline ForRangedNode()
		{
			inputs =
			{
				AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(ForRangedNode::ForRange)),
				AttributeConfig<int32_t>("First Index", AttributeDirection::Input),
				AttributeConfig<int32_t>("Last Index", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig("", AttributeDirection::Output),
				AttributeConfig("Iteration", AttributeDirection::Output),
				AttributeConfig<int32_t>("Index", AttributeDirection::Output, true)
			};
		}

		inline const std::string GetName() override { return "For Loop"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void ForRange()
		{
			const auto firstIndex = GetInput<int32_t>(1);
			const auto lastIndex = GetInput<int32_t>(2);
			for (int32_t i = firstIndex; i <= lastIndex; i++)
			{
				SetOutputData(2, i);
				ActivateOutput(1);
			}

			ActivateOutput(0);
		}
	};

	class FlipFlopNode : public Node
	{
	public:
		inline FlipFlopNode()
		{
			inputs =
			{
				AttributeConfig("", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig("A", AttributeDirection::Output),
				AttributeConfig("B", AttributeDirection::Output),

				AttributeConfig<bool>("Is A", AttributeDirection::Output, true, GK_BIND_FUNCTION(FlipFlopNode::FlipFlop))
			};
		}

		inline const std::string GetName() override { return "Flip Flop"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void FlipFlop()
		{
			myState = !myState;

			SetOutputData(2, myState);
			if (myState)
			{
				ActivateOutput(0);
			}
			else
			{
				ActivateOutput(1);
			}
		}

		bool myState = false;
	};

	class NotNode : public Node
	{
	public:
		inline NotNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<bool>("Value", AttributeDirection::Input, true)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(NotNode::Not))
			};
		}

		inline const std::string GetName() override { return "Not"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void Not()
		{
			const auto rhs = GetInput<bool>(0);

			SetOutputData(0, !rhs);
		}
	};

	class AndNode : public Node
	{
	public:
		inline AndNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<bool>("A", AttributeDirection::Input, true),
				AttributeConfig<bool>("B", AttributeDirection::Input, true)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(AndNode::And))
			};
		}

		inline const std::string GetName() override { return "And"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void And()
		{
			SetOutputData(0, GetInput<bool>(0) && GetInput<bool>(1));
		}
	};

	class OrNode : public Node
	{
	public:
		inline OrNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<bool>("A", AttributeDirection::Input, true),
				AttributeConfig<bool>("B", AttributeDirection::Input, true)
			};

			outputs =
			{
				AttributeConfig<bool>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(OrNode::Or))
			};
		}

		inline const std::string GetName() override { return "Or"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void Or()
		{
			SetOutputData(0, GetInput<bool>(0) || GetInput<bool>(1));
		}
	};
}
