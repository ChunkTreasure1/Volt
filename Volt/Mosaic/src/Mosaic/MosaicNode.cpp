#include "mcpch.h"
#include "MosaicNode.h"

namespace Mosaic
{
	MosaicNode::MosaicNode(MosaicGraph* ownerGraph)
		: m_graph(ownerGraph)
	{
	}

	void MosaicNode::AddInputParameter(const std::string& name, ValueBaseType baseType, uint32_t vectorSize, bool showAttribute)
	{
		auto& param = m_inputParameters.emplace_back();
		param.name = name;
		param.typeInfo.baseType = baseType;
		param.typeInfo.vectorSize = vectorSize;
		param.direction = ParameterDirection::Input;
		param.index = static_cast<uint32_t>(m_inputParameters.size() - 1);
		param.showAttribute = showAttribute;
	}

	void MosaicNode::AddOutputParameter(const std::string& name, ValueBaseType baseType, uint32_t vectorSize, bool showAttribute)
	{
		auto& param = m_outputParameters.emplace_back();
		param.name = name;
		param.typeInfo.baseType = baseType;
		param.typeInfo.vectorSize = vectorSize;
		param.direction = ParameterDirection::Output;
		param.index = static_cast<uint32_t>(m_outputParameters.size() - 1);
		param.showAttribute = showAttribute;
	}

	Parameter& MosaicNode::GetInputParameter(uint32_t parameterIndex)
	{
		return m_inputParameters.at(parameterIndex);
	}

	Parameter& MosaicNode::GetOutputParameter(uint32_t parameterIndex)
	{
		return m_outputParameters.at(parameterIndex);
	}

	const Parameter& MosaicNode::GetInputParameter(uint32_t parameterIndex) const
	{
		return m_inputParameters.at(parameterIndex);
	}

	const Parameter& MosaicNode::GetOutputParameter(uint32_t parameterIndex) const
	{
		return m_outputParameters.at(parameterIndex);
	}
}
