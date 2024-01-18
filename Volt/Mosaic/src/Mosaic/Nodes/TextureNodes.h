#pragma once

#include "Mosaic/MosaicNode.h"

namespace Mosaic
{
	class SampleTextureNode : public MosaicNode
	{
	public:
		SampleTextureNode(MosaicGraph* ownerGraph);
		~SampleTextureNode() override;

		inline const std::string GetName() const override { return "Sample Texture"; }
		inline const std::string GetCategory() const override { return "Texture"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return "{DB60F69D-EFC5-4AA4-BF5A-C89D58942D3F}"_guid; }

		void Reset() override;

		const ResultInfo GetShaderCode(const GraphNode<Ref<class MosaicNode>, Ref<MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override;

	private:
		uint32_t m_textureIndex = 0;

		bool m_evaluated = false;
	    ResultInfo m_evaluatedResultInfo;
	};
}
