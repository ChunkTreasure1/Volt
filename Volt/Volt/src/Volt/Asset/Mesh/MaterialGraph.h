#pragma once

#include <GraphKey/Graph.h>

namespace Volt
{
	class MaterialGraphAsset : public GraphKey::Graph
	{
	public:
		inline MaterialGraphAsset() = default;
		~MaterialGraphAsset() override = default;

		inline void SetState(const std::string& state) { myGraphState = state; }
		inline const std::string& GetState() const { return myGraphState; }

		static AssetType GetStaticType() { return Volt::AssetType::MaterialGraph; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		std::string myGraphState;
	};
}