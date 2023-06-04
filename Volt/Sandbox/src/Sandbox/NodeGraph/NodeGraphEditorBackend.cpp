#include "sbpch.h"
#include "NodeGraphEditorBackend.h"

namespace NodeGraph
{
	const NodeGraph::Link EditorBackend::CreateLink(const Volt::UUID inputId, const Volt::UUID outputId)
	{
		NodeGraph::Link newLink{};
		newLink.input = outputId;
		newLink.output = inputId;

		links.emplace_back(newLink);

		return newLink;
	}
}

