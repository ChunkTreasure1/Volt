#pragma once

#include "GraphKey/Node.h"

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
}