#pragma once

#include <Mosaic/MosaicNode.h>

namespace Volt::MosaicNodes
{
	class AddNode : public Mosaic::MosaicNode
	{
	public:
		AddNode(Mosaic::MosaicGraph* ownerGraph);

		inline const std::string GetName() const override { return "Add"; }
		inline const std::string GetCategory() const override { return "Math"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return "{08434406-3093-4AA2-B3B8-2B39AA0EE744}"_guid; }

		const Mosaic::ResultInfo GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override;

	private:
	};
}
