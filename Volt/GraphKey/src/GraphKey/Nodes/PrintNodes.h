#pragma once

#include "GraphKey/Node.h"

#include <Volt/Log/Log.h>

namespace GraphKey
{
	class PrintNode : public Node
	{
	public:
		PrintNode()
		{
			inputs =
			{
				AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(PrintNode::Print)),
				AttributeConfig<std::string>("String", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig("", AttributeDirection::Output, nullptr)
			};
		}

		inline const std::string GetName() override { return "Print"; }
		inline const gem::vec4 GetColor() override { return { 1.f, 0.f, 0.f, 1.f }; }

	private:
		inline void Print()
		{
			const std::string value = GetInput<std::string>(1);
			VT_CORE_INFO(value);
			ActivateOutput(0);
		}
	};
}