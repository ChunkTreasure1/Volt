#include "sbpch.h"
#include "GraphKeyPanel.h"

#include "Sandbox/Window/GraphKey/PinDrawing.h"
#include "Sandbox/Window/GraphKey/GraphKeyHelpers.h"

#include "Sandbox/Utility/SelectionManager.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Utility/UIUtility.h>

#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>
#include <GraphKey/Registry.h>

#include <builders.h>

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

template<typename T>
inline constexpr std::type_index GetTypeIndex()
{
	return std::type_index(typeid(T));
}

GraphKeyPanel::GraphKeyPanel(Ref<Volt::Scene>& aScene)
	: EditorWindow("Graph Key", true), myCurrentScene(aScene)
{
	myInstance = this;

	CreateAttributeFunctions();
	CreateAttributeColors();

	InitializeEditor();
}

GraphKeyPanel::~GraphKeyPanel()
{
	ed::DestroyEditor(myEditorContext);
	myInstance = nullptr;
}

void GraphKeyPanel::UpdateMainContent()
{}

void GraphKeyPanel::UpdateContent()
{
	ed::SetCurrentEditor(myEditorContext);

	UpdateNodesPanel();
	UpdatePropertiesPanel();
	UpdateEditorPanel();

	ed::SetCurrentEditor(nullptr);
}

void GraphKeyPanel::SetActiveGraph(Ref<GraphKey::Graph> graph)
{
	myCurrentGraph = graph;
	InitializeEditor();
}

void GraphKeyPanel::UpdateEditorPanel()
{
	ImGui::Begin("Editor##graphKey", nullptr);

	ed::Begin("Editor##graphKey");

	if (myCurrentGraph)
	{
		for (const auto& node : myCurrentGraph->GetSpecification().nodes)
		{
			DrawNode(node);
		}
		
		for (const auto& link : myCurrentGraph->GetSpecification().links)
		{
			ed::Link(ed::LinkId(link->id), ed::PinId(link->output), ed::PinId(link->input));
		}

		if (ed::BeginCreate({ 1.f, 1.f, 1.f, 1.f }, 2.f))
		{
			ed::PinId startPinId = 0, endPinId = 0;
			if (ed::QueryNewLink(&startPinId, &endPinId))
			{
				auto* startAttr = myCurrentGraph->GetAttributeByID(startPinId.Get());
				auto* endAttr = myCurrentGraph->GetAttributeByID(endPinId.Get());

				if (startAttr && endAttr)
				{
					if (startAttr->direction == GraphKey::AttributeDirection::Input)
					{
						std::swap(startAttr, endAttr);
						std::swap(startPinId, endPinId);
					}

					bool sameType = false;
					if (startAttr->type == GraphKey::AttributeType::Flow && endAttr->type == GraphKey::AttributeType::Flow)
					{
						sameType = true;
					}

					if (!sameType)
					{
						if (startAttr->data.has_value() && endAttr->data.has_value())
						{
							sameType = startAttr->data.type() == endAttr->data.type();
						}
					}

					if (startAttr->linkable && endAttr->linkable && ed::AcceptNewItem() && sameType && startAttr->direction != endAttr->direction)
					{
						myCurrentGraph->CreateLink(endAttr->id, startAttr->id);
					}
				}
			}
		}
		ed::EndCreate();

		ed::Suspend();
		if (ed::ShowNodeContextMenu(&myContextNodeId))
		{
			UI::OpenPopup("NodeContextMenu");
		}
		else if (ed::ShowPinContextMenu(&myContextPinId))
		{
			UI::OpenPopup("PinContextMenu");
		}
		else if (ed::ShowLinkContextMenu(&myContextLinkId))
		{
			UI::OpenPopup("LinkContextMenu");
		}
		else if (ed::ShowBackgroundContextMenu())
		{
			UI::OpenPopup("BackgroundContextMenu");
		}

		UpdateContextPopups();
		ed::Resume();
	}


	ed::End();
	ImGui::End();
}

void GraphKeyPanel::UpdateContextPopups()
{
	if (UI::BeginPopup("NodeContextMenu"))
	{
		ImGui::MenuItem("Delete");

		UI::EndPopup();
	}

	if (UI::BeginPopup("PinContextMenu"))
	{
		GraphKey::Attribute* pin = myCurrentGraph->GetAttributeByID(myContextPinId.Get());
		if (pin->data.has_value())
		{
			const bool isEntity = pin->data.type() == GetTypeIndex<Volt::Entity>();
			if (isEntity)
			{
				if (SelectionManager::IsAnySelected() && ImGui::MenuItem("Assign Selected Entity"))
				{
					pin->data = Volt::Entity{ SelectionManager::GetSelectedEntities().at(0), myCurrentScene.get() };
				}

				//if (ImGui::MenuItem("Assign Current Entity"))
				//{
				//	pin->data = 
				//}
			}
		}

		UI::EndPopup();
	}

	if (UI::BeginPopup("LinkContextMenu"))
	{
		ImGui::MenuItem("Delete");

		UI::EndPopup();
	}

	bool isNewNodePoppedUp = false;

	static ImVec2 newNodePostion{ 0.0f, 0.0f };

	if (UI::BeginPopup("BackgroundContextMenu"))
	{
		isNewNodePoppedUp = true;
		newNodePostion = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup());

		Ref<GraphKey::Node> node;

		for (const auto& [name, func] : GraphKey::Registry::GetRegistry())
		{
			if (ImGui::MenuItem(name.c_str()))
			{
				node = func();
				myCurrentGraph->AddNode(node);
			}
		}

		if (node)
		{
			ed::SetNodePosition(ed::NodeId(node->id), newNodePostion);
			ImGui::CloseCurrentPopup();
		}

		UI::EndPopup();
	}
}

void GraphKeyPanel::InitializeEditor()
{
	if (myEditorContext)
	{
		ed::DestroyEditor(myEditorContext);
	}

	ax::NodeEditor::Config cfg{};
	cfg.SettingsFile = nullptr;
	cfg.UserPointer = this;

	cfg.SaveSettings = [](const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer)
	{
		GraphKeyPanel* panel = static_cast<GraphKeyPanel*>(userPointer);
		return panel->SetNodeSettings(data);
	};

	cfg.LoadSettings = [](char* data, void* userPointer) -> size_t
	{
		GraphKeyPanel* panel = static_cast<GraphKeyPanel*>(userPointer);
		const std::string graphContext = panel->GetNodeSettings();

		if (data)
		{
			memcpy_s(data, graphContext.size(), graphContext.c_str(), graphContext.size());
		}

		return graphContext.size();
	};

	myEditorContext = ed::CreateEditor(&cfg);
	ed::SetCurrentEditor(myEditorContext);
	ed::EnableShortcuts(true);
}

void GraphKeyPanel::CreateAttributeFunctions()
{
	myAttributeFunctions[GetTypeIndex<bool>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<bool&>(value)); };
	myAttributeFunctions[GetTypeIndex<int32_t>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<int32_t&>(value)); };
	myAttributeFunctions[GetTypeIndex<uint32_t>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<uint32_t&>(value)); };
	myAttributeFunctions[GetTypeIndex<int16_t>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<int16_t&>(value)); };
	myAttributeFunctions[GetTypeIndex<uint16_t>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<uint16_t&>(value)); };
	myAttributeFunctions[GetTypeIndex<int8_t>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<int8_t&>(value)); };
	myAttributeFunctions[GetTypeIndex<uint8_t>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<uint8_t&>(value)); };
	myAttributeFunctions[GetTypeIndex<double>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<double&>(value)); };
	myAttributeFunctions[GetTypeIndex<float>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<float&>(value)); };
	myAttributeFunctions[GetTypeIndex<gem::vec2>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec2&>(value)); };
	myAttributeFunctions[GetTypeIndex<gem::vec3>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec3&>(value)); };
	myAttributeFunctions[GetTypeIndex<gem::vec4>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec4&>(value)); };
	myAttributeFunctions[GetTypeIndex<gem::vec2ui>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec2ui&>(value)); };
	myAttributeFunctions[GetTypeIndex<gem::vec3ui>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec3ui&>(value)); };
	myAttributeFunctions[GetTypeIndex<gem::vec4ui>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<gem::vec4ui&>(value)); };
	myAttributeFunctions[GetTypeIndex<std::string>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<std::string&>(value)); };

	myAttributeFunctions[GetTypeIndex<Volt::Entity>()] = [](std::any& value) { GraphKeyHelpers::Attribute(std::any_cast<Volt::Entity&>(value)); };
}

void GraphKeyPanel::CreateAttributeColors()
{
	///// Colors //////
	// Bool: Dark red
	// Int: Green
	// Float: Blue
	// String: Magenta
	// VectorN: Yellow
	// Default: Orange
	///////////////////

	myAttributeColors[GetTypeIndex<bool>()] = { 0.58f, 0.f, 0.01f, 1.f };

	myAttributeColors[GetTypeIndex<int32_t>()] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<uint32_t>()] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<int16_t>()] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<uint16_t>()] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<int8_t>()] = { 0.12f, 0.72f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<uint8_t>()] = { 0.12f, 0.72f, 0.f, 1.f };

	myAttributeColors[GetTypeIndex<double>()] = { 0.15f, 0.29f, 0.83f, 1.f };
	myAttributeColors[GetTypeIndex<float>()] = { 0.15f, 0.29f, 0.83f, 1.f };

	myAttributeColors[GetTypeIndex<gem::vec2>()] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<gem::vec3>()] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<gem::vec4>()] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<gem::quat>()] = { 0.96f, 0.99f, 0.f, 1.f };

	myAttributeColors[GetTypeIndex<gem::vec2ui>()] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<gem::vec3ui>()] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<gem::vec4ui>()] = { 0.96f, 0.99f, 0.f, 1.f };
	myAttributeColors[GetTypeIndex<std::string>()] = { 0.96f, 0.99f, 0.f, 1.f };

	myAttributeColors[GetTypeIndex<Volt::Entity>()] = { 0.3f, 1.f, 0.49f, 1.f };

	myDefaultPinColor = { 0.99f, 0.51f, 0.f, 1.f };
}

void GraphKeyPanel::UpdateNodesPanel()
{
	ImGui::Begin("Nodes Panel");

	for (const auto& [name, func] : GraphKey::Registry::GetRegistry())
	{
		if (ImGui::Button(name.c_str()) && myCurrentGraph)
		{
			myCurrentGraph->AddNode(GraphKey::Registry::Create(name));
		}
	}

	if (ImGui::Button("TestPlay")) //#TODO_Ivar: Change how this works
	{
		for (const auto& node : myCurrentGraph->GetSpecification().nodes)
		{
			if (node->GetName() == "Start")
			{
				Volt::OnScenePlayEvent e{};
				node->OnEvent(e);
				break;
			}
		}
	}

	ImGui::End();
}

void GraphKeyPanel::DrawNode(Ref<GraphKey::Node> node)
{
	ImTextureID textureId = nullptr;
	int32_t width = 0;
	int32_t height = 0;

	const Ref<Volt::Texture2D> headerTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Graph/Translucency.dds");
	if (headerTexture && headerTexture->IsValid())
	{
		textureId = UI::GetTextureID(headerTexture);
		width = headerTexture->GetWidth();
		height = headerTexture->GetHeight();
	}

	utils::BlueprintNodeBuilder builder{ textureId, width, height };

	builder.Begin(ed::NodeId(node->id));

	{
		const auto color = node->GetColor();

		builder.Header(ImColor{ color.x, color.y, color.z, color.w });
		{
			ImGui::Spring(0.f);
			ImGui::TextUnformatted(node->GetName().c_str());
		}
		ImGui::Spring(1.f);

		const float nodeHeaderHeight = 18.0f;
		ImGui::Dummy(ImVec2(0, nodeHeaderHeight));

		ImGui::Spring(0);
		builder.EndHeader();
	}

	GraphKeyHelpers::BeginAttributes();

	for (auto& input : node->inputs)
	{
		const auto typeIndex = std::type_index(input.data.type());
		builder.Input(ed::PinId(input.id));

		const bool connected = !input.links.empty();

		gem::vec4 color{};
		if (input.type == GraphKey::AttributeType::Flow)
		{
			color = { 1.f, 1.f, 1.f, 1.f };
		}
		else
		{
			if (myAttributeColors.contains(typeIndex))
			{
				color = myAttributeColors.at(typeIndex);
			}
			else
			{
				color = myDefaultPinColor;
			}
		}

		if (input.linkable)
		{
			DrawPinIcon(input, connected, ImColor{ color.x, color.y, color.z, color.w }, 255);
		}

		ImGui::Spring(0.f);
		ImGui::TextUnformatted(input.name.c_str());

		if (!input.inputHidden && !connected && input.data.has_value())
		{
			if (myAttributeFunctions.contains(typeIndex))
			{
				myAttributeFunctions.at(typeIndex)(input.data);
			}
		}

		ImGui::Spring(0.f);

		builder.EndInput();
	}

	for (auto& output : node->outputs)
	{
		const auto typeIndex = std::type_index(output.data.type());
		builder.Output(ed::PinId(output.id));

		const bool connected = !output.links.empty();

		ImGui::Spring(0.f);

		if (!output.inputHidden && !connected && output.data.has_value())
		{
			if (myAttributeFunctions.contains(typeIndex))
			{
				myAttributeFunctions.at(typeIndex)(output.data);
			}
		}

		ImGui::TextUnformatted(output.name.c_str());
		ImGui::Spring(0.f);

		gem::vec4 color{};

		if (output.type == GraphKey::AttributeType::Flow)
		{
			color = { 1.f, 1.f, 1.f, 1.f };
		}
		else
		{
			if (myAttributeColors.contains(typeIndex))
			{
				color = myAttributeColors.at(typeIndex);
			}
			else
			{
				color = myDefaultPinColor;
			}
		}


		if (output.linkable)
		{
			DrawPinIcon(output, connected, ImColor{ color.x, color.y, color.z, color.w }, 255);
		}

		builder.EndOutput();
	}

	GraphKeyHelpers::EndAttributes();

	builder.End();
}

bool GraphKeyPanel::SetNodeSettings(const char* data)
{
	if (!myCurrentGraph)
	{
		return false;
	}

	auto entityId = myCurrentGraph->GetEntity();
	Volt::Entity entity = { entityId, myCurrentScene.get() };

	if (!entity)
	{
		return false;
	}

	if (!entity.HasComponent<Volt::VisualScriptingComponent>())
	{
		return false;
	}

	auto& vsComp = entity.GetComponent<Volt::VisualScriptingComponent>();
	vsComp.graphState = data;

	return true;
}

const std::string GraphKeyPanel::GetNodeSettings() const
{
	if (!myCurrentGraph)
	{
		return {};
	}

	auto entityId = myCurrentGraph->GetEntity();
	Volt::Entity entity = { entityId, myCurrentScene.get() };

	if (!entity)
	{
		return {};
	}

	if (!entity.HasComponent<Volt::VisualScriptingComponent>())
	{
		return {};
	}

	auto& vsComp = entity.GetComponent<Volt::VisualScriptingComponent>();
	return vsComp.graphState;
}

void GraphKeyPanel::UpdatePropertiesPanel()
{
	ImGui::Begin("Properties##Nodes");

	ImGui::End();
}

const std::vector<Ref<GraphKey::Node>> GraphKeyPanel::GetSelectedNodes() const
{
	std::vector<ed::NodeId> selectedNodeIds;
	selectedNodeIds.resize(ed::GetSelectedObjectCount());

	int32_t nodeCount = ed::GetSelectedNodes(selectedNodeIds.data(), (int32_t)selectedNodeIds.size());
	selectedNodeIds.resize(nodeCount);

	std::vector<Ref<GraphKey::Node>> selectedNodes;
	for (const auto& node : myCurrentGraph->GetSpecification().nodes)
	{
		auto it = std::find_if(selectedNodeIds.begin(), selectedNodeIds.end(), [&](const ed::NodeId& id)
			{
				return id.Get() == node->id;
			});

		if (it != selectedNodeIds.end())
		{
			selectedNodes.emplace_back(node);
		}
	}

	return selectedNodes;
}

const std::vector<Ref<GraphKey::Link>> GraphKeyPanel::GetSelectedLinks() const
{
	std::vector<ed::LinkId> selectedLinkIds;
	selectedLinkIds.resize(ed::GetSelectedObjectCount());

	int32_t linkCount = ed::GetSelectedLinks(selectedLinkIds.data(), (int32_t)selectedLinkIds.size());
	selectedLinkIds.resize(linkCount);

	std::vector<Ref<GraphKey::Link>> selectedLinks;
	for (const auto& link : myCurrentGraph->GetSpecification().links)
	{
		auto it = std::find_if(selectedLinkIds.begin(), selectedLinkIds.end(), [&](const ed::LinkId& id)
			{
				return id.Get() == link->id;
			});

		if (it != selectedLinkIds.end())
		{
			selectedLinks.emplace_back(link);
		}
	}

	return selectedLinks;
}