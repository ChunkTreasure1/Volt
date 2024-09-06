#pragma once

#include <Mosaic/MosaicNode.h>
#include <Mosaic/MosaicGraph.h>
#include <Mosaic/MosaicHelpers.h>

#include <Mosaic/FormatterExtension.h>

namespace Volt::MosaicNodes
{
	template<typename ValueType, ValueType DEFAULT_VALUE, Mosaic::ValueBaseType BASE_TYPE, VoltGUID GUID>
	class MakeVec3Node : public Mosaic::MosaicNode
	{
	public:
		MakeVec3Node(Mosaic::MosaicGraph* ownerGraph)
			: Mosaic::MosaicNode(ownerGraph)
		{
			AddInputParameter("R", BASE_TYPE, 1, DEFAULT_VALUE, true);
			AddInputParameter("G", BASE_TYPE, 1, DEFAULT_VALUE, true);
			AddInputParameter("B", BASE_TYPE, 1, DEFAULT_VALUE, true);
		
			AddOutputParameter("Result", BASE_TYPE, 3, DEFAULT_VALUE, false);
		}

		inline const std::string GetName() const override { return "Make " + Mosaic::Helpers::GetTypeNameFromTypeInfo(TYPE_INFO); }
		inline const std::string GetCategory() const override { return "Conversion"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return GUID; }

		inline const Mosaic::ResultInfo GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override
		{
			constexpr const char* nodeStr = "const {0} {1} = {2}({3}, {4}, {5});";
		
			std::string R = std::to_string(GetInputParameter(0).Get<ValueType>());
			std::string G = std::to_string(GetInputParameter(1).Get<ValueType>());
			std::string B = std::to_string(GetInputParameter(2).Get<ValueType>());

			for (const auto& edgeId : underlyingNode.GetInputEdges())
			{
				const auto edge = underlyingNode.GetEdgeFromID(edgeId);
				const uint32_t paramIndex = edge.metaDataType->GetParameterInputIndex();

				const auto& node = underlyingNode.GetNodeFromID(edge.startNode);
				const Mosaic::ResultInfo info = node.nodeData->GetShaderCode(node, edge.metaDataType->GetParameterOutputIndex(), appendableShaderString);

				if (paramIndex == 0)
				{
					R = info.resultParamName;
				}
				else if (paramIndex == 1)
				{
					G = info.resultParamName;
				}
				else if (paramIndex == 2)
				{
					B = info.resultParamName;
				}
			}

			const std::string varName = m_graph->GetNextVariableName();
			
			const auto typeString = Mosaic::Helpers::GetTypeNameFromTypeInfo(TYPE_INFO);
			std::string result = std::format(nodeStr, typeString, varName, typeString, R, G, B);
			appendableShaderString.append(result);

			Mosaic::ResultInfo resultInfo{};
			resultInfo.resultParamName = varName;
			resultInfo.resultType = TYPE_INFO;

			return resultInfo;
		}

	private:
		inline static constexpr Mosaic::TypeInfo TYPE_INFO{ BASE_TYPE, 3 };
	};
}
