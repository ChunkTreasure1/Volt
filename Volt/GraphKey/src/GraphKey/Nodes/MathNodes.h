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

		~AddNode() override = default;

		inline const std::string GetName() override { return "Add"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

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

		~SubtractNode() override = default;

		inline const std::string GetName() override { return "Subtract"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

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
				AttributeConfig<T>("Result", AttributeDirection::Output, true, [this]()
				{
					const T a = GetInput<T>(0);
					const T b = GetInput<T>(1);

					const T val = a * b;
					SetOutputData(0, val);
				})
			};
		}

		~MultiplyNode() override = default;

		inline const std::string GetName() override { return "Multiply"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }
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

		~DivisionNode() override = default;

		inline const std::string GetName() override { return "Division"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

	private:
		inline void Divide()
		{
			const T val = GetInput<T>(0) / GetInput<T>(1);
			SetOutputData(0, val);
		}
	};

	template<typename T, uint32_t N>
	class BreakVectorNode : public Node
	{
	public:
		inline BreakVectorNode()
		{
			constexpr const char* components[] = { "X", "Y", "Z", "W" };

			inputs =
			{
				AttributeConfig<T>("Input", AttributeDirection::Input, true),
			};

			for (uint32_t i = 0; i < N; i++)
			{
				auto attr = AttributeConfig<float>(components[i], AttributeDirection::Output, true, GK_BIND_FUNCTION(BreakVectorNode::Decompose));
				outputs.push_back(attr);
			}
		}

		~BreakVectorNode() override = default;

		inline const std::string GetName() override { return "Break Vector"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }

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
	class MakeVectorNode : public Node
	{
	public:
		inline MakeVectorNode()
		{
			constexpr const char* components[] = { "X", "Y", "Z", "W" };

			for (uint32_t i = 0; i < N; i++)
			{
				auto attr = AttributeConfig<float>(components[i], AttributeDirection::Input);
				inputs.push_back(attr);
			}

			outputs =
			{
				AttributeConfig<T>("Result", AttributeDirection::Output, true, GK_BIND_FUNCTION(MakeVectorNode::Compose))
			};
		}

		~MakeVectorNode() override = default;

		inline const std::string GetName() override { return "Make Vector"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }
	private:
		inline void Compose()
		{
			T result{};

			for (uint32_t i = 0; i < N; i++)
			{
				result[i] = GetInput<float>(i);
			}

			SetOutputData(0, result);
		}
	};

	template<typename T>
	class NormalizeVectorNode : public Node
	{
	public:
		inline NormalizeVectorNode()
		{
			inputs =
			{
				AttributeConfig<T>("Input", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<T>("Output", AttributeDirection::Output, true, GK_BIND_FUNCTION(NormalizeVectorNode::Normalize))
			};
		}

		~NormalizeVectorNode() override = default;

		inline const std::string GetName() override { return "Normalize"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }
	private:
		inline void Normalize()
		{
			T result = GetInput<T>(0);

			if (result != T(0.f))
			{
				SetOutputData(0, glm::normalize(result));
			}
			else
			{
				SetOutputData(0, result);
			}
		}
	};

	template<typename T>
	class LengthVectorNode : public Node
	{
	public:
		inline LengthVectorNode()
		{
			inputs =
			{
				AttributeConfig<T>("Input", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<float>("Output", AttributeDirection::Output, true, GK_BIND_FUNCTION(LengthVectorNode::Length))
			};
		}

		~LengthVectorNode() override = default;

		inline const std::string GetName() override { return "Length"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }
	private:
		inline void Length()
		{
			T result = GetInput<T>(0);

			if (result != T(0.f))
			{
				SetOutputData(0, glm::length(result));
			}
			else
			{
				SetOutputData(0, 0.f);
			}
		}
	};

	class SlerpNode : public Node
	{
	public:
		inline SlerpNode()
		{
			inputs =
			{
				AttributeConfig<glm::vec3>("A", AttributeDirection::Input),
				AttributeConfig<glm::vec3>("B", AttributeDirection::Input),
				AttributeConfig<float>("t", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<glm::vec3>("Output", AttributeDirection::Output, true, GK_BIND_FUNCTION(SlerpNode::Slerp))
			};
		}

		~SlerpNode() override = default;

		inline const std::string GetName() override { return "Slerp"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }
	private:

		void Slerp();
	};

	class LerpNode : public Node
	{
	public:
		inline LerpNode()
		{
			inputs =
			{
				AttributeConfig<glm::vec3>("A", AttributeDirection::Input),
				AttributeConfig<glm::vec3>("B", AttributeDirection::Input),
				AttributeConfig<float>("t", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<glm::vec3>("Output", AttributeDirection::Output, true, GK_BIND_FUNCTION(LerpNode::Lerp))
			};
		}

		~LerpNode() override = default;

		inline const std::string GetName() override { return "Lerp"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 1.f, 0.f, 1.f }; }
	private:

		void Lerp();
	};
	
	template<typename T, typename L>
	struct ConvertFromToNode : public Node
	{ 
		ConvertFromToNode()
		{
			isHeaderless = true;

			inputs =
			{
				AttributeConfig<T>("A", AttributeDirection::Input),
			};

			outputs =
			{
				AttributeConfig<L>("B", AttributeDirection::Output, true, GK_BIND_FUNCTION(ConvertFromToNode::Convert))
			};
		}

		~ConvertFromToNode() override = default;

		inline const std::string GetName() override { return "To"; }
		inline const glm::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void Convert()
		{
			const auto lhs = GetInput<T>(0);

			SetOutputData(0, (L)lhs);
		}
	};
}
