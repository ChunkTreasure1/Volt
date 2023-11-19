#pragma once

#include <Volt/Core/UUID.h>
#include <Volt/Asset/Asset.h>

#include <vector>

namespace NodeGraph
{
	struct Link
	{
		UUID64 input = 0;
		UUID64 output = 0;

		UUID64 id{};
	};

	struct Node
	{
		UUID64 id{};
		std::vector<UUID64> pins;
	};

	struct EditorBackend
	{
		virtual ~EditorBackend() = default;
		virtual const Volt::AssetType GetSupportedAssetTypes() { return Volt::AssetType::None; }
		const NodeGraph::Link CreateLink(const UUID64 inputId, const UUID64 outputId);

		std::vector<Node> nodes;
		std::vector<Link> links;
	};
}
