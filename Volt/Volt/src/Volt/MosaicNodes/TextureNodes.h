#pragma once

#include "Volt/Asset/Asset.h"

#include <Mosaic/MosaicNode.h>

namespace Volt
{
	struct TextureInfo
	{
		uint32_t textureIndex;
		AssetHandle textureHandle;
	};

	class SampleTextureNode : public Mosaic::MosaicNode
	{
	public:
		SampleTextureNode(Mosaic::MosaicGraph* ownerGraph);
		~SampleTextureNode() override;

		inline const std::string GetName() const override { return "Sample Texture"; }
		inline const std::string GetCategory() const override { return "Texture"; }
		inline const glm::vec4 GetColor() const override { return 1.f; }
		inline const VoltGUID GetGUID() const override { return "{DB60F69D-EFC5-4AA4-BF5A-C89D58942D3F}"_guid; }

		void Reset() override;
		void RenderCustomWidget() override;
		void SerializeCustom(YAMLStreamWriter& streamWriter) const override;
		void DeserializeCustom(YAMLStreamReader& streamReader) override;

		const Mosaic::ResultInfo GetShaderCode(const GraphNode<Ref<class Mosaic::MosaicNode>, Ref<Mosaic::MosaicEdge>>& underlyingNode, uint32_t outputIndex, std::string& appendableShaderString) const override;

		const TextureInfo GetTextureInfo() const;

	private:
		uint32_t m_textureIndex = 0;
		AssetHandle m_textureHandle = Asset::Null();

		mutable bool m_evaluated = false;
		mutable Mosaic::ResultInfo m_evaluatedResultInfo;
	};
}
