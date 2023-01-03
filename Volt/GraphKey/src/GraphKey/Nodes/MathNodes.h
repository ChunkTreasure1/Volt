#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	template<typename T>
	class AddNode : public Node
	{
	public:
		inline AddNode()
		{
			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<T>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(AddNode::Add))
			};
		}

		inline const std::string GetName() override { return "Add"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Add()
		{
			const T val = GetInput<T>(0) + GetInput<T>(1);
			SetOutputData(0, val);
		}
	};

	template<typename T>
	class SubtractNode : public Node
	{
	public:
		inline SubtractNode()
		{
			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<T>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(SubtractNode::Subtract))
			};
		}

		inline const std::string GetName() override { return "Subtract"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Subtract()
		{
			const T val = GetInput<T>(0) - GetInput<T>(1);
			SetOutputData(0, val);
		}
	};

	template<typename T>
	class MultiplyNode : public Node
	{
	public:
		inline MultiplyNode()
		{
			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<T>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(MultiplyNode::Multiply))
			};
		}

		inline const std::string GetName() override { return "Multiply"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Multiply()
		{
			const T val = GetInput<T>(0) * GetInput<T>(1);
			SetOutputData(0, val);
		}
	};

	template<typename T>
	class DivisionNode : public Node
	{
	public:
		inline DivisionNode()
		{
			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
				AttributeConfig<T>("B", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<T>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(DivisionNode::Divide))
			};
		}

		inline const std::string GetName() override { return "Division"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Divide()
		{
			const T val = GetInput<T>(0) / GetInput<T>(1);
			SetOutputData(0, val);
		}
	};

	template<typename T, uint32_t N>
	class DecomposeVectorNode : public Node
	{
	public:
		inline DecomposeVectorNode()
		{
			constexpr const char* components[] = { "X", "Y", "Z", "W" };

			inputs =
			{
				AttributeConfig<T>("Input", AttributeDirection::Input, true),
			};

			for (uint32_t i = 0; i < N; i++)
			{
				auto attr = AttributeConfig<float>(components[i], AttributeDirection::Output, true, GK_BIND_FUNCTION(DecomposeVectorNode::Decompose));
				outputs.push_back(attr);
			}
		}

		inline const std::string GetName() override { return "Decompose Vector"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Decompose()
		{
			const T input = GetInput<T>(0);

			for (uint32_t i = 0; i < N; i++)
			{
				SetOutputData(i, input[i]);
			}
		}
	};

	template<typename T, uint32_t N>
	class ComposeVectorNode : public Node
	{
	public:
		inline ComposeVectorNode()
		{
			constexpr const char* components[] = { "X", "Y", "Z", "W" };
			
			for (uint32_t i = 0; i < N; i++)
			{
				auto attr = AttributeConfig<float>(components[i], AttributeDirection::Input);
				inputs.push_back(attr);
			}

			outputs =
			{
				AttributeConfig<T>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(ComposeVectorNode::Compose))
			};
		}

		inline const std::string GetName() override { return "Compose Vector"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }
	private:
		inline void Compose()
		{
			T result;

			for (uint32_t i = 0; i < N; i++)
			{
				result[i] = GetInput<float>(i);
			}

			SetOutputData(0, result);
		}
	};
}