#include "sbpch.h"
#include "MaterialGraphPanel.h"

#include "Volt/Asset/Mesh/MaterialGraph.h"

MaterialGraphPanel::MaterialGraphPanel(Ref<Volt::Scene>& currentScene)
	: IONodeGraphEditor("Material Graph", "Material Graph", currentScene)
{
	myInstance = this;

	myOpenMaterialGraph = CreateRef<Volt::MaterialGraphAsset>();
	myOpenGraph = myOpenMaterialGraph;
	ReconstructGraph();
}

MaterialGraphPanel::~MaterialGraphPanel()
{
	myInstance = nullptr;
}

void MaterialGraphPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	myOpenMaterialGraph = std::reinterpret_pointer_cast<Volt::MaterialGraphAsset>(asset);
	myOpenGraph = std::reinterpret_pointer_cast<GraphKey::Graph>(asset);
	ReconstructGraph();
}

bool MaterialGraphPanel::SaveSettings(const std::string & data)
{
	if (!myOpenMaterialGraph)
	{
		return false;
	}

	myOpenMaterialGraph->SetState(data);
	return true;
}

size_t MaterialGraphPanel::LoadSettings(std::string& data)
{
	if (!myOpenMaterialGraph)
	{
		return 0;
	}

	const auto& state = myOpenMaterialGraph->GetState();
	data = state;

	return data.size();
}

bool MaterialGraphPanel::SaveNodeSettings(const Volt::UUID nodeId, const std::string& data)
{
	auto node = myOpenGraph->GetNodeByID(nodeId);
	if (!node)
	{
		return false;
	}

	node->editorState = data;
	return true;
}

size_t MaterialGraphPanel::LoadNodeSettings(const Volt::UUID nodeId, std::string& data)
{
	auto node = myOpenGraph->GetNodeByID(nodeId);
	if (!node)
	{
		return false;
	}

	data = node->editorState;
	return data.size();
}
