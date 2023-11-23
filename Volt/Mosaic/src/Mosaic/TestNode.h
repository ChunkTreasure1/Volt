#pragma once

#include "MosaicNode.h"

namespace Mosaic
{
	class TestAddNode : public MosaicNode
	{
	public:
		inline const std::string GetName() const override { return "TestAdd"; }
		inline const std::string GetCategory() const override { return "Math"; }
		inline const VoltGUID GetGUID() const override { return "{4560882A-DF27-4486-B332-5E791D7B6D71}"_guid; }

		const std::string GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode) const override;
	};
}
