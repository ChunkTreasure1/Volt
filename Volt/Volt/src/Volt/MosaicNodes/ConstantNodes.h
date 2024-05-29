#pragma once

#include <Mosaic/MosaicNode.h>
#include <Mosaic/MosaicGraph.h>
#include <Mosaic/MosaicHelpers.h>

#include <Mosaic/FormatterExtension.h>

#include <imgui.h>

namespace Volt::MosaicNodes
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
				resultInfo.resultType = TYPE_INFO;

				return resultInfo;
			}

			const std::string varName = m_graph->GetNextVariableName();
			std::string result = std::format(nodeStr, Mosaic::Helpers::GetTypeNameFromTypeInfo(TYPE_INFO), varName, GetOutputParameter(0).Get<ValueType>());
			appendableShaderString.append(result);

			Mosaic::ResultInfo resultInfo{};
			resultInfo.resultParamName = varName;
			resultInfo.resultType = TYPE_INFO;

			m_evaluatedVariableName = varName;
			m_evaluated = true;

			return resultInfo;
		}

	private:
		inline static constexpr Mosaic::TypeInfo TYPE_INFO{ BASE_TYPE, VECTOR_SIZE };

		mutable bool m_evaluated = false;
		mutable std::string m_evaluatedVariableName;
	};

	static void GetCorrectedColorVariableName(Mosaic::ResultInfo& resultInfo, uint32_t outputIndex, uint32_t vectorSize)
	{
		if (outputIndex == 0)
		{
			resultInfo.resultType.vectorSize = vectorSize;
		}
		else if (outputIndex == 1)
		{
			resultInfo.resultParamName += ".r";
		}
		else if (outputIndex == 2)
		{
			resultInfo.resultParamName += ".g";
		}
		else if (outputIndex == 3)
		{
			resultInfo.resultParamName += ".b";
		}

		if (vectorSize == 4 && outputIndex == 4)
		{
			resultInfo.resultParamName += ".a";
		}
	}

	template<typename ValueType, ValueType DEFAULT_VALUE, uint32_t VECTOR_SIZE, VoltGUID GUID>
	class ColorNode : public Mosaic::MosaicNode
	{
	public:
		ColorNode(Mosaic::MosaicGraph* ownerGraph)
			: Mosaic::MosaicNode(ownerGraph)
		{ 
			if constexpr (VECTOR_SIZE == 3)
			{
				AddOutputParameter("RGB", Mosaic::ValueBaseType::Float, VECTOR_SIZE, DEFAULT_VALUE, false);
			}
			else
			{
				AddOutputParameter("RGBA", Mosaic::ValueBaseType::Float, VECTOR_SIZE, DEFAULT_VALUE, false);
			}

			AddOutputParameter("R", Mosaic::ValueBaseType::Float, VECTOR_SIZE, DEFAULT_VALUE, false);
			AddOutputParameter("G", Mosaic::ValueBaseType::Float, VECTOR_SIZE, DEFAULT_VALUE, false);
			AddOutputParameter("B", Mosaic::ValueBaseType::Float, VECTOR_SIZE, DEFAULT_VALUE, false);

			if constexpr (VECTOR_SIZE == 4)
			{
				AddOutputParameter("A", Mosaic::ValueBaseType::Float, VECTOR_SIZE, DEFAULT_VALUE, false);
			}
		}

		VT_INLINE const std::string GetName() const override { return "Color" + std::to_string(VECTOR_SIZE); }
		VT_INLINE const std::string GetCategory() const override { return "Constants"; }
		VT_INLINE const glm::vec4 GetColor() const override { return 1.f; }
		VT_INLINE const VoltGUID GetGUID() const override { return GUID; }

		VT_INLINE void Reset() override
		{
			m_evaluated = false;
		}

		VT_INLINE const Mosaic::ResultInfo GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override
		{
			constexpr const char* nodeStr = "const {0} {1} = {2}; \n";

			if (m_evaluated)
			{
				Mosaic::ResultInfo resultInfo{};
				resultInfo.resultParamName = m_evaluatedVariableName;
				resultInfo.resultType = Mosaic::TypeInfo{ Mosaic::ValueBaseType::Float, 1 };

				GetCorrectedColorVariableName(resultInfo, outputIndex, VECTOR_SIZE);

				return resultInfo;
			}

			const std::string varName = m_graph->GetNextVariableName();
			std::string result = std::format(nodeStr, Mosaic::Helpers::GetTypeNameFromTypeInfo(TYPE_INFO), varName, GetOutputParameter(0).Get<ValueType>());
			appendableShaderString.append(result);

			Mosaic::ResultInfo resultInfo{};
			resultInfo.resultParamName = varName;
			resultInfo.resultType = Mosaic::TypeInfo{ Mosaic::ValueBaseType::Float, 1 };

			m_evaluatedVariableName = varName;
			m_evaluated = true;

			GetCorrectedColorVariableName(resultInfo, outputIndex, VECTOR_SIZE);

			return resultInfo;
		}

		VT_INLINE void RenderCustomWidget() override
		{
			if constexpr (VECTOR_SIZE == 3)
			{
				ImGui::ColorEdit3("##colorEdit3", glm::value_ptr(GetOutputParameter(0).Get<ValueType>()));
			}
			else
			{
				ImGui::ColorEdit4("##colorEdit4", glm::value_ptr(GetOutputParameter(0).Get<ValueType>()));
			}
		}

	private:
		inline static constexpr Mosaic::TypeInfo TYPE_INFO{ Mosaic::ValueBaseType::Float, VECTOR_SIZE };

		mutable bool m_evaluated = false;
		mutable std::string m_evaluatedVariableName;
	};
}
