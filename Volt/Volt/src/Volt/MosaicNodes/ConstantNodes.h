#pragma once

#include <Mosaic/MosaicNode.h>
#include <Mosaic/MosaicGraph.h>
#include <Mosaic/MosaicHelpers.h>

#include <Mosaic/FormatterExtension.h>

namespace Volt
{
	template<typename ValueType, ValueType DEFAULT_VALUE, Mosaic::ValueBaseType BASE_TYPE, uint32_t VECTOR_SIZE, VoltGUID GUID>
	class ConstantNode : public Mosaic::MosaicNode
	{
	public:
		ConstantNode(Mosaic::MosaicGraph* ownerGraph)
			: Mosaic::MosaicNode(ownerGraph)
		{
			AddOutputParameter("Value", BASE_TYPE, VECTOR_SIZE, DEFAULT_VALUE, true);
		}

		inline const std::string GetName() const override { return Mosaic::Helpers::GetTypeNameFromTypeInfo(TYPE_INFO) + " Constant"; }
		inline const std::string GetCategory() const override { return "Constants"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return GUID; }

		inline void Reset() override 
		{
			m_evaluated = false;
		}

		inline const Mosaic::ResultInfo GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override
		{
			constexpr const char* nodeStr = "const {0} {1} = {2}; \n";

			if (m_evaluated)
			{
				Mosaic::ResultInfo resultInfo{};
				resultInfo.resultParamName = m_evaluatedVariableName;
				resultInfo.resultType = Mosaic::TypeInfo{ BASE_TYPE, VECTOR_SIZE };

				return resultInfo;
			}

			const std::string varName = m_graph->GetNextVariableName();
			std::string result = std::format(nodeStr, Mosaic::Helpers::GetTypeNameFromTypeInfo(TYPE_INFO), varName, GetOutputParameter(0).Get<ValueType>());
			appendableShaderString.append(result);

			Mosaic::ResultInfo resultInfo{};
			resultInfo.resultParamName = varName;
			resultInfo.resultType = Mosaic::TypeInfo{ BASE_TYPE, VECTOR_SIZE };
			
			const_cast<std::string&>(m_evaluatedVariableName) = varName;
			const_cast<bool&>(m_evaluated) = true;

			return resultInfo;
		}

	private:
		inline static constexpr Mosaic::TypeInfo TYPE_INFO{ BASE_TYPE, VECTOR_SIZE };

		bool m_evaluated = false;
		std::string m_evaluatedVariableName;
	};
}
