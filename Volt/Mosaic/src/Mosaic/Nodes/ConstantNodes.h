#pragma once

#include "Mosaic/MosaicNode.h"
#include "Mosaic/MosaicGraph.h"
#include "Mosaic/MosaicHelpers.h"

#include "Mosaic/FormatterExtension.h"

namespace Mosaic
{
	//class FloatConstantNode : public MosaicNode
	//{
	//public:
	//	FloatConstantNode();

	//	inline const std::string GetName() const override { return "Float Constant"; }
	//	inline const std::string GetCategory() const override { return "Constants"; }
	//	inline const VoltGUID GetGUID() const override { return "{D4FD37DA-6B0D-4F92-A055-AE37A6FC80AB}"_guid; }

	//	const std::string GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode) const override;
	//};

	template<typename ValueType, ValueBaseType BASE_TYPE, uint32_t VECTOR_SIZE, VoltGUID GUID>
	class ConstantNode : public MosaicNode
	{
	public:
		ConstantNode(MosaicGraph* ownerGraph)
			: MosaicNode(ownerGraph)
		{
			AddOutputParameter("Value", BASE_TYPE, VECTOR_SIZE);
		}

		inline const std::string GetName() const override { return Helpers::GetTypeNameFromTypeInfo(TYPE_INFO) + " Constant"; }
		inline const std::string GetCategory() const override { return "Constants"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return GUID; }

		inline void Reset() override 
		{
			m_evaluated = false;
		}

		inline const ResultInfo GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override
		{
			constexpr const char* nodeStr = "const {0} {1} = {2}; \n";

			if (m_evaluated)
			{
				ResultInfo resultInfo{};
				resultInfo.resultParamName = m_evaluatedVariableName;
				resultInfo.resultType = TypeInfo{ BASE_TYPE, VECTOR_SIZE };

				return resultInfo;
			}

			const std::string varName = m_graph->GetNextVariableName();
			std::string result = std::format(nodeStr, Helpers::GetTypeNameFromTypeInfo(TYPE_INFO), varName, GetOutputParameter(0).Get<ValueType>());
			appendableShaderString.append(result);

			ResultInfo resultInfo{};
			resultInfo.resultParamName = varName;
			resultInfo.resultType = TypeInfo{ BASE_TYPE, VECTOR_SIZE };
			
			const_cast<std::string&>(m_evaluatedVariableName) = varName;
			const_cast<bool&>(m_evaluated) = true;

			return resultInfo;
		}

	private:
		inline static constexpr TypeInfo TYPE_INFO{ BASE_TYPE, VECTOR_SIZE };

		bool m_evaluated = false;
		std::string m_evaluatedVariableName;
	};
}
