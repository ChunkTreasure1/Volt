#pragma once

#include "Mosaic/MosaicNode.h"

namespace Mosaic
{
	class PBROutputNode : public MosaicNode
	{
	public:
		PBROutputNode(MosaicGraph* ownerGraph);

		inline const std::string GetName() const override { return "PBR Output"; }
		inline const std::string GetCategory() const override { return "Output"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return "{343B2C0A-C4E3-41BB-8629-F9939795AC76}"_guid; }

		const ResultInfo GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override;

	private:
	};
}
