#pragma once

#include <GraphKey/Graph.h>

namespace Volt
{
	class GraphKeyAsset : public GraphKey::Graph
	{
	public:
		static Volt::AssetType GetStaticType() { return Volt::AssetType::GraphKey; }
		Volt::AssetType GetType() override { return GetStaticType(); };
	};
}