#pragma once

#include "Mosaic/MosaicNode.h"

namespace Mosaic
{
	class AddNode : public MosaicNode
	{
	public:
		AddNode(MosaicGraph* ownerGraph);

		inline const std::string GetName() const override { return "Add"; }
		inline const std::string GetCategory() const override { return "Math"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return "{08434406-3093-4AA2-B3B8-2B39AA0EE744}"_guid; }

		const ResultInfo GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override;

	private:
	};
}
