#pragma once

#include "Mosaic/Parameter.h"

#include <CoreUtilities/Core.h>
#include <CoreUtilities/VoltGUID.h>
#include <CoreUtilities/Containers/Graph.h>

#include <vector>

namespace Mosaic
{
	// #TODO_Ivar: Move to another file
	class MosaicEdge
	{
	public:
	};

	class MosaicNode
	{
	public:
		virtual const std::string GetName() const = 0;
		virtual const std::string GetCategory() const = 0;
		virtual const VoltGUID GetGUID() const = 0;

		virtual const std::string GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode) const = 0;
		inline const std::vector<InputParameter> GetParameters() { return m_parameters; }
		inline const ReturnValue GetReturnValue() { return m_returnValue; }

	protected:
		std::vector<InputParameter> m_parameters;
		ReturnValue m_returnValue;
	};
}
