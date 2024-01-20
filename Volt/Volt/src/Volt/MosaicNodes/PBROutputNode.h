#pragma once

#include <Mosaic/MosaicNode.h>

namespace Volt
{
	class PBROutputNode : public Mosaic::MosaicNode
	{
	public:
		PBROutputNode(Mosaic::MosaicGraph* ownerGraph);

		inline const std::string GetName() const override { return "PBR Output"; }
		inline const std::string GetCategory() const override { return "Output"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return "{343B2C0A-C4E3-41BB-8629-F9939795AC76}"_guid; }

		const Mosaic::ResultInfo GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override;

	private:
	};
}
