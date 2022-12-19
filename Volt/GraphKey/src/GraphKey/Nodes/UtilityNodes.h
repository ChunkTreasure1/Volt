#pragma once

#include "GraphKey/Node.h"
#include "GraphKey/Registry.h"

#include <Volt/Events/ApplicationEvent.h>

namespace GraphKey
{
	template<typename T>
	class ToStringNode : public Node
	{
	public:
		inline ToStringNode()
		{
			inputs =
			{
				AttributeConfig<T>("", GraphKey::AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig<std::string>("Output", GraphKey::AttributeDirection::Output, true, GK_BIND_FUNCTION(ToStringNode::ToString))
			};
		}

		inline const std::string GetName() override { return "To String"; }
		inline const gem::vec4 GetColor() override { return { 1.f, 0.f, 0.f, 1.f }; }

	private:
		void ToString()
		{
			const auto val = GetInput<T>(0);
			SetOutputData(0, std::to_string(val));
		}
	};

	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeFloat, ToStringNode<float>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeDouble, ToStringNode<double>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt32, ToStringNode<int32_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt32, ToStringNode<uint32_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt16, ToStringNode<int16_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt16, ToStringNode<uint16_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt8, ToStringNode<int8_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt8, ToStringNode<uint8_t>);
}