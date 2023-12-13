#include "sbpch.h"
#include "GraphKeyPanel.h"

GraphKeyPanel::GraphKeyPanel(Ref<Volt::Scene>& currentScene)
	: IONodeGraphEditor("Graph Key", "Graph Key", currentScene)
{
	myInstance = this;
}

GraphKeyPanel::~GraphKeyPanel()
{
	myInstance = nullptr;
}

void GraphKeyPanel::OpenAsset(Ref<Volt::Asset> asset)
{
}

bool GraphKeyPanel::SaveSettings(const std::string& data) 
{
	if (!myOpenGraph)
	{
		return false;
	}

	auto entityId = myOpenGraph->GetEntity();
	Volt::Entity entity = myCurrentScene->GetEntityFromUUID(entityId);

	if (!entity)
	{
		return false;
	}

	//if (!entity.HasComponent<Volt::VisualScriptingComponent>())
	//{
	//	return false;
	//}

	//auto& vsComp = entity.GetComponent<Volt::VisualScriptingComponent>();
	//vsComp.graphState = data;

	return true;
}

size_t GraphKeyPanel::LoadSettings(std::string& data) 
{
	if (!myOpenGraph)
	{
		return 0;
	}

	auto entityId = myOpenGraph->GetEntity();
	Volt::Entity entity = myCurrentScene->GetEntityFromUUID(entityId);

	if (!entity)
	{
		return 0;
	}

	//if (!entity.HasComponent<Volt::VisualScriptingComponent>())
	//{
	//	return 0;
	//}

	//auto& vsComp = entity.GetComponent<Volt::VisualScriptingComponent>();
	//data = vsComp.graphState;

	return data.size();
}

bool GraphKeyPanel::SaveNodeSettings(const Volt::UUID nodeId, const std::string& data) 
{
	auto node = myOpenGraph->GetNodeByID(nodeId);
	if (!node)
	{
		return false;
	}

	node->editorState = data;
	return true;
}

size_t GraphKeyPanel::LoadNodeSettings(const Volt::UUID nodeId, std::string& data) 
{
	auto node = myOpenGraph->GetNodeByID(nodeId);
	if (!node)
	{
		return false;
	}

	data = node->editorState;
	return data.size();
}

void GraphKeyPanel::SetActiveGraph(Ref<GraphKey::Graph> graph)
{
	myOpenGraph = graph;
	ReconstructGraph();
}
