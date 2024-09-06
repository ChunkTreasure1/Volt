#pragma once

#include <Volt/Core/UUID.h>
#include <AssetSystem/Asset.h>

#include <CoreUtilities/Containers/Vector.h>

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
		Vector<UUID64> pins;
	};

	struct EditorBackend
	{
		virtual ~EditorBackend() = default;
		virtual const AssetType GetSupportedAssetTypes() { return AssetTypes::None; }
		const NodeGraph::Link CreateLink(const UUID64 inputId, const UUID64 outputId);

		Vector<Node> nodes;
		Vector<Link> links;
	};
}
