#include "gkpch.h"
#include "PrintNodes.h"

namespace GraphKey
{
	PrintNode::PrintNode()
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

	void PrintNode::Print()
	{
		const std::string value = GetInput<std::string>(1);
		VT_LOG(LogSeverity::Info, value);
		ActivateOutput(0);
	}
}
