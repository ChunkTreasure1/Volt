#include "gkpch.h"
#include "BaseMaterialNodes.h"

namespace GraphKey
{
	MaterialOutputNode::MaterialOutputNode()
	{
		inputs =
		{
			AttributeConfig<gem::vec4>("Base Color", AttributeDirection::Input),
			AttributeConfig<float>("Metallic", AttributeDirection::Input),
			AttributeConfig<float>("Roughness", AttributeDirection::Input),
			AttributeConfig<gem::vec3>("Emissive Color", AttributeDirection::Input),
			AttributeConfig<gem::vec3>("Normal", AttributeDirection::Input),
			AttributeConfig<gem::vec3>("World Position Offset", AttributeDirection::Input),
		};
	}

	const std::string MaterialOutputNode::Evaluate(uint32_t pin) const
	{


		return std::string();
	}

	const std::string MaterialOutputNode::CompileFunction() const
	{
		return std::string();
	}

	TextureSampleNode::TextureSampleNode()
	{
		inputs =
		{
			AttributeConfig<gem::vec2>("UVs", AttributeDirection::Input),
		};

		outputs =
		{
			AttributeConfig<gem::vec3>("RGB", AttributeDirection::Output),
			AttributeConfig<float>("R", AttributeDirection::Output),
			AttributeConfig<float>("G", AttributeDirection::Output),
			AttributeConfig<float>("B", AttributeDirection::Output),
			AttributeConfig<float>("A", AttributeDirection::Output),
			AttributeConfig<gem::vec4>("RGBA", AttributeDirection::Output)
		};
	}

	TextureSampleNode::~TextureSampleNode()
	{
	}

	const std::string TextureSampleNode::Evaluate(uint32_t pin) const
	{
		std::string result;
		std::string uvString = "input.texCoords";

		if (!inputs[0].links.empty())
		{
			const auto link = myGraph->GetLinkByID(inputs[0].links.at(0));
			const auto node = myGraph->GetNodeFromAttributeID(link->output);
			const auto index = node->GetAttributeIndexFromID(link->output);
			uvString = std::reinterpret_pointer_cast<MaterialBaseNode>(node)->Evaluate(index);
		}

		switch (pin)
		{
			case 0: result = std::format("{0}.rgb", ""); break;
			case 1: result = std::format("{0}.r", ""); break;
			case 2: result = std::format("{0}.g", ""); break;
			case 3: result = std::format("{0}.b", ""); break;
			case 4: result = std::format("{0}.a", ""); break;
			case 5: result = std::format("{0}.rgba", ""); break;
		}

		return result;
	}
	const std::string TextureSampleNode::CompileFunction() const
	{
		const char* function =
			R"(
				float4 Sample_{$textureName}(float2 uv)
				{
					return {$textureName}.Sample(u_linearSampler, uv);
				}
			)";

		return function;
	}
}
