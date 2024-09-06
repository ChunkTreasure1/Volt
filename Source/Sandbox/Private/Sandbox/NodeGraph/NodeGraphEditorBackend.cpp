#include "sbpch.h"
#include "NodeGraph/NodeGraphEditorBackend.h"

namespace NodeGraph
{
	const NodeGraph::Link EditorBackend::CreateLink(const UUID64 inputId, const UUID64 outputId)
	{
		NodeGraph::Link newLink{};
		newLink.input = outputId;
		newLink.output = inputId;

		links.emplace_back(newLink);

		return newLink;
	}
}

