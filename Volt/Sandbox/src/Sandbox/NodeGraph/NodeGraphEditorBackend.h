#pragma once

#include <Volt/Core/UUID.h>
#include <Volt/Asset/Asset.h>

#include <vector>

namespace NodeGraph
{
	struct Link
	{
		Volt::UUID input = 0;
		Volt::UUID output = 0;

		Volt::UUID id{};
	};

	struct Node
	{
		Volt::UUID id{};
		std::vector<Volt::UUID> pins;
	};

	struct EditorBackend
	{
		virtual ~EditorBackend() = default;
		virtual const Volt::AssetType GetSupportedAssetTypes() { return Volt::AssetType::None; }
		const NodeGraph::Link CreateLink(const Volt::UUID inputId, const Volt::UUID outputId);

		std::vector<Node> nodes;
		std::vector<Link> links;
	};
}