#pragma once

#include "Mosaic/Parameter.h"

#include <CoreUtilities/Core.h>
#include <CoreUtilities/VoltGUID.h>
#include <CoreUtilities/Containers/Graph.h>

#include <glm/glm.hpp>

#include <vector>

namespace Mosaic
{
	// #TODO_Ivar: Move to another file
	class MosaicEdge 
	{
	public:
		MosaicEdge(uint32_t parameterInputIndex, uint32_t parameterOutputIndex)
			: m_parameterInputIndex(parameterInputIndex), m_parameterOutputIndex(parameterOutputIndex)
		{ }

		inline const uint32_t GetParameterInputIndex() const { return m_parameterInputIndex; }
		inline const uint32_t GetParameterOutputIndex() const { return m_parameterOutputIndex; }

	private:
		uint32_t m_parameterInputIndex = 0;
		uint32_t m_parameterOutputIndex = 0;
	};

	class MosaicGraph;

	class MosaicNode
	{
	public:
		MosaicNode(MosaicGraph* ownerGraph);
		virtual ~MosaicNode() {}

		virtual const std::string GetName() const = 0;
		virtual const std::string GetCategory() const = 0;
		virtual const glm::vec4 GetColor() const = 0;
		virtual const VoltGUID GetGUID() const = 0;
		virtual void Reset() {}

		virtual const ResultInfo GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const = 0;

		inline const std::vector<Parameter>& GetInputParameters() const { return m_inputParameters; }
		inline const std::vector<Parameter>& GetOutputParameters() const { return m_outputParameters; }

		inline std::vector<Parameter>& GetInputParameters() { return m_inputParameters; }
		inline std::vector<Parameter>& GetOutputParameters() { return m_outputParameters; }

		Parameter& GetInputParameter(uint32_t parameterIndex);
		Parameter& GetOutputParameter(uint32_t parameterIndex);

		const Parameter& GetInputParameter(uint32_t parameterIndex) const;
		const Parameter& GetOutputParameter(uint32_t parameterIndex) const;

		inline std::string& GetEditorState() { return m_editorState; }

	protected:
		void AddInputParameter(const std::string& name, ValueBaseType baseType, uint32_t vectorSize);
		void AddOutputParameter(const std::string& name, ValueBaseType baseType, uint32_t vectorSize);

		MosaicGraph* m_graph = nullptr;

	private:
		std::vector<Parameter> m_inputParameters;
		std::vector<Parameter> m_outputParameters;

		std::string m_editorState;
	};
}
