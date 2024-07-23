#pragma once

#include "Sandbox/NodeGraph/NodeGraphEditor.h"
#include "Sandbox/NodeGraph/IONodeGraphEditorHelpers.h"
#include "Sandbox/NodeGraph/NodeGraphEditorPinUtility.h"
#include "Sandbox/NodeGraph/NodeGraphEditorBackend.h"
#include "Sandbox/NodeGraph/NodeGraphCommandStack.h"

#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"

#include <Volt/Asset/Importers/TextureImporter.h>
#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

#include <Volt/Events/KeyEvent.h>
#include <Volt/Input/Input.h>
#include <Volt/Input/KeyCodes.h>

#include <GraphKey/Registry.h>
#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>
#include <GraphKey/Nodes/ParameterNodes.h>
#include <GraphKey/Nodes/CustomEventNode.h>
#include <GraphKey/TypeTraits.h>

#include <imgui_internal.h>
#include <builders.h>
#include <typeindex>

namespace GraphKey
{
	class Graph;
	struct Attribute;
}

namespace Volt
{
	class Texture2D;
}

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

namespace Utility
{
	inline Ref<GraphKey::Node> CreateNodeFromParameterName(const std::string& paramName, Ref<GraphKey::Graph> graph)
	{
		const std::string pName = paramName.substr(paramName.find_first_of(' ') + 1);

		const auto& blackboard = graph->GetBlackboard();
		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&](const auto& lhs)
		{
			return pName == lhs.name;
		});

		if (it == blackboard.end())
		{
			return nullptr;
		}

		const auto& param = *it;

		const auto splitParam = Utility::SplitStringsByCharacter(paramName, ' ');
		if (splitParam.size() < 2)
		{
			return nullptr;
		}

		if (Utility::StringContains(Utility::ToLower(splitParam.at(0)), "set"))
		{
			Ref<GraphKey::Node> node;
			GK_CREATE_SET_PARAMETER_NODE(std::type_index(param.value.type()), param.id, node);

			return node;
		}
		else
		{
			Ref<GraphKey::Node> node;
			GK_CREATE_GET_PARAMETER_NODE(std::type_index(param.value.type()), param.id, node);

			return node;
		}
	}

	inline Ref<GraphKey::Node> CreateCallNodeFromEventName(const std::string& nodeName, Volt::Entity pinEntity)
	{
		/*if (!pinEntity.HasComponent<Volt::VisualScriptingComponent>())
		{
			return nullptr;
		}

		auto& comp = pinEntity.GetComponent<Volt::VisualScriptingComponent>();
		if (!comp.graph)
		{
			return nullptr;
		}

		const auto& events = comp.graph->GetEvents();
		const std::string eventName = nodeName.substr(nodeName.find_first_of(' ') + 1);

		auto it = std::find_if(events.begin(), events.end(), [&eventName](const auto& lhs)
		{
			return lhs.name == eventName;
		});

		if (it == events.end())
		{
			return nullptr;
		}

		Ref<GraphKey::Node> newNode = GraphKey::Registry::Create("CallCustomEventNode");
		auto callType = std::reinterpret_pointer_cast<GraphKey::CallCustomEventNode>(newNode);
		callType->eventId = (*it).id;

		return newNode;*/
	
		return nullptr;
	}

	inline Ref<GraphKey::Node> CreateRecieveNodeFromEventName(const std::string& nodeName, Ref<GraphKey::Graph> graph, Ref<Volt::Scene> current)
	{
		//auto entId = graph->GetEntity();
		//Volt::Entity entity{ entId, current.get() };

		//if (!entity.HasComponent<Volt::VisualScriptingComponent>())
		//{
		//	return nullptr;
		//}

		//auto& comp = entity.GetComponent<Volt::VisualScriptingComponent>();
		//if (!comp.graph)
		//{
		//	return nullptr;
		//}

		//const auto& events = comp.graph->GetEvents();
		//const std::string eventName = nodeName.substr(nodeName.find_first_of(' ') + 1);

		//auto it = std::find_if(events.begin(), events.end(), [&eventName](const auto& lhs)
		//{
		//	return lhs.name == eventName;
		//});

		//if (it == events.end())
		//{
		//	return nullptr;
		//}

		//Ref<GraphKey::Node> newNode = GraphKey::Registry::Create("RecieveCustomEventNode");
		//auto callType = std::reinterpret_pointer_cast<GraphKey::RecieveCustomEventNode>(newNode);
		//callType->eventId = (*it).id;

		//return newNode;

		return nullptr;
	}
}


template<GraphKey::GraphType TGraphType, typename EditorBackend>
class IONodeGraphEditor : public NodeGraph::Editor
{
public:
	enum class IncompatiblePinReason
	{
		None = 0,
		IncompatibleType,
		IncompatibleDirection,
		SamePin,
		NonLinkable
	};

	IONodeGraphEditor(const std::string& title, const std::string& context, Ref<Volt::Scene>& currentScene);
	~IONodeGraphEditor() override = default;

	void OnEvent(Volt::Event& e) override;

protected:
	void DrawNodes() override;
	void DrawPanels() override;
	void DrawContextPopups() override;
	void ReconstructGraph();

	virtual void DrawNodesPanel();
	virtual void OnBeginCreate();
	virtual void OnDeleteLink(const UUID64 id);
	virtual void OnDeleteNode(const UUID64 id);

	virtual void OnCopy();
	virtual void OnPaste();

	Ref<GraphKey::Graph> myOpenGraph;
	Ref<Volt::Scene>& myCurrentScene;

private:
	const IncompatiblePinReason CanLinkPins(const UUID64 input, const UUID64 output);

	void AddNode(Ref<GraphKey::Node> node);
	void CreateLink(const UUID64 id, const UUID64 input, const UUID64 output) override;
	void AddLink(GraphKey::Link link);

	void RemoveLink(const UUID64 linkId) override;
	void RemoveNode(const UUID64 nodeId) override;

	void DrawGraphDataPanel();
	void DrawNodeContextMenu();
	void DrawPinContextMenu();
	void DrawLinkContextMenu();
	void DrawBackgroundContextMenu();

	void InitializeStyle(ed::Style& editorStyle) override;

	const Volt::Entity GetEntityFromCurrentNewPin();

	const Vector<std::string> GetEventNamesFromCurrentNewPin();

	const glm::vec4 GetColorFromAttribute(const GraphKey::Attribute& attr);
	void DrawPinIcon(const GraphKey::Attribute& pin, bool connected, ImColor color, int32_t alpha);
	Ref<GraphKey::Node> DrawNodeList(std::string& query, std::type_index typeIndex = std::type_index{ typeid(void) });

	Ref<Volt::Texture2D> myHeaderTexture;
	Scope<NodeGraphCommandStack> myCommandStack;

	const GraphKey::GraphType myGraphType = TGraphType;
	UUID64 myNewNodeLinkPinId = UUID64(0);

	std::string mySearchQuery;
	std::string myContextSearchQuery;

	Vector<Ref<GraphKey::Node>> myNodeCopies;
	Vector<GraphKey::Link> myLinkCopies;

	bool myShouldMoveCopies = false;
	bool myActivateSearchWidget = false;
};

template<GraphKey::GraphType graphType, typename EditorBackend>
inline IONodeGraphEditor<graphType, EditorBackend>::IONodeGraphEditor(const std::string& title, const std::string& context, Ref<Volt::Scene>& currentScene)
	: NodeGraph::Editor(title, context, true, CreateRef<EditorBackend>()), myCurrentScene(currentScene)
{
	if (myGraphType == GraphKey::GraphType::Animation)
	{
		SetGraphTypeText("ANIMATION");
	}
	else if (myGraphType == GraphKey::GraphType::Scripting)
	{
		SetGraphTypeText("GRAPHKEY");
	}

	myHeaderTexture = CreateRef<Volt::Texture2D>();
	Volt::TextureImporter::ImportTexture("Editor/Textures/Graph/Translucency.dds", *myHeaderTexture);

	{
		Ref<NodeGraph::EditorContext> editorContext = CreateRef<NodeGraph::EditorContext>();

		editorContext->onAcceptCopy = [&]()
		{
			OnCopy();
		};

		editorContext->onAcceptPaste = [&]()
		{
			OnPaste();
		};

		editorContext->onBeginCreate = [&]()
		{
			OnBeginCreate();
		};

		editorContext->onDeleteLink = [this](const UUID64 id)
		{
			OnDeleteLink(id);
		};

		editorContext->onDeleteNode = [this](const UUID64 id)
		{
			OnDeleteNode(id);
		};

		editorContext->onShowNodeContextMenu = []()
		{
			UI::OpenPopup("NodeContextMenu");
		};

		editorContext->onShowPinContextMenu = []()
		{
			UI::OpenPopup("PinContextMenu");
		};

		editorContext->onShowLinkContextMenu = []()
		{
			UI::OpenPopup("LinkContextMenu");
		};

		editorContext->onShowBackgroundContextMenu = [&]()
		{
			myActivateSearchWidget = true;
			UI::OpenPopup("BackgroundContextMenu");
		};

		InitializeEditor(editorContext);
	}

	myCommandStack = CreateScope<NodeGraphCommandStack>();
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::KeyPressedEvent>([&](auto& e)
	{
		const bool ctrlPressed = Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL);

		if (e.GetKeyCode() == VT_KEY_Z && ctrlPressed)
		{
			myCommandStack->Undo(myOpenGraph, [&]() { ReconstructGraph(); });
		}
		else if (e.GetKeyCode() == VT_KEY_Y && ctrlPressed)
		{
			myCommandStack->Redo(myOpenGraph, [&]() { ReconstructGraph(); });
		}
		else if (e.GetKeyCode() == VT_KEY_DELETE)
		{
			myCommandStack->AddCommand<NodeGraphGraphKeyCommand>(myOpenGraph);
		}
		return false;
	});
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawNodes()
{
	ImTextureID textureId = nullptr;
	int32_t width = 0;
	int32_t height = 0;

	if (myHeaderTexture->IsValid())
	{
		textureId = UI::GetTextureID(myHeaderTexture);
		width = myHeaderTexture->GetWidth();
		height = myHeaderTexture->GetHeight();
	}

	utils::BlueprintNodeBuilder builder{ textureId, width, height };

	for (const auto& n : GetBackend().nodes)
	{
		auto graphNode = myOpenGraph->GetNodeByID(n.id);

		builder.Begin(ed::NodeId(n.id));

		if (!graphNode->isHeaderless)
		{
			const auto color = graphNode->GetColor();

			builder.Header(ImColor{ color.x, color.y, color.z, color.w });
			{
				ImGui::Spring(0.f);
				ImGui::TextUnformatted(graphNode->GetName().c_str());
			}
			ImGui::Spring(1.f);

			const float nodeHeaderHeight = 18.0f;
			ImGui::Dummy(ImVec2(0, nodeHeaderHeight));

			ImGui::Spring(0);
			builder.EndHeader();
		}

		IONodeGraphEditorHelpers::BeginAttributes();

		for (auto& input : graphNode->inputs)
		{
			float alpha = ImGui::GetStyle().Alpha;
			ed::PinId linkStartPin = { myNewLinkPinId };
			ed::PinId inputPin = { input.id };

			const auto reason = CanLinkPins(linkStartPin.Get(), inputPin.Get());
			if (myNewLinkPinId && reason != IncompatiblePinReason::None && input.id != myNewLinkPinId)
			{
				alpha = alpha * (48.f / 255.f);
			}

			const auto typeIndex = std::type_index(input.data.type());

			UI::ScopedStyleFloat alphaStyle{ ImGuiStyleVar_Alpha, alpha };
			builder.Input(ed::PinId(input.id));

			const bool connected = !input.links.empty();
			glm::vec4 color = GetColorFromAttribute(input);

			if (input.linkable)
			{
				DrawPinIcon(input, connected, ImColor{ color.x, color.y, color.z, color.w }, (int32_t)(alpha * 255.f));
			}

			ImGui::Spring(0.f);
			ImGui::TextUnformatted(input.name.c_str());

			if (!input.inputHidden && !connected && input.data.has_value())
			{
				const auto& attribFuncs = IONodeGraphEditorHelpers::GetAttribFunctions();

				if (input.data.type() == typeid(Volt::AssetHandle))
				{
					IONodeGraphEditorHelpers::Attribute(std::any_cast<Volt::AssetHandle&>(input.data), GetBackend().GetSupportedAssetTypes());
				}
				else
				{
					if (attribFuncs.contains(typeIndex))
					{
						attribFuncs.at(typeIndex)(input.data);
					}
				}
			}

			ImGui::Spring(0.f);

			builder.EndInput();
		}

		if (graphNode->isHeaderless)
		{
			builder.Middle();

			ImGui::Spring(1, 0);
			ImGui::TextUnformatted(graphNode->GetName().c_str());
			ImGui::Spring(1, 0);
		}

		for (auto& output : graphNode->outputs)
		{
			float alpha = ImGui::GetStyle().Alpha;
			ed::PinId linkStartPin{ myNewLinkPinId };
			ed::PinId outputPin{ output.id };

			const auto reason = CanLinkPins(linkStartPin.Get(), outputPin.Get());
			if (myNewLinkPinId && reason != IncompatiblePinReason::None && output.id != myNewLinkPinId)
			{
				alpha = alpha * (48.f / 255.f);
			}

			const auto typeIndex = std::type_index(output.data.type());

			UI::ScopedStyleFloat alphaStyle{ ImGuiStyleVar_Alpha, alpha };

			builder.Output(ed::PinId(output.id));

			const bool connected = !output.links.empty();

			ImGui::Spring(0.f);

			if (!output.inputHidden && output.data.has_value())
			{
				const auto& attribFuncs = IONodeGraphEditorHelpers::GetAttribFunctions();

				if (output.data.type() == typeid(Volt::AssetHandle))
				{
					IONodeGraphEditorHelpers::Attribute(std::any_cast<Volt::AssetHandle&>(output.data), GetBackend().GetSupportedAssetTypes());
				}
				else
				{
					if (attribFuncs.contains(typeIndex))
					{
						attribFuncs.at(typeIndex)(output.data);
					}
				}
			}

			ImGui::TextUnformatted(output.name.c_str());
			ImGui::Spring(0.f);

			glm::vec4 color = GetColorFromAttribute(output);

			if (output.linkable)
			{
				DrawPinIcon(output, connected, ImColor{ color.x, color.y, color.z, color.w }, (int32_t)(alpha * 255.f));
			}

			builder.EndOutput();
		}

		IONodeGraphEditorHelpers::EndAttributes();

		builder.End();
	}

	if (myShouldMoveCopies)
	{
		myShouldMoveCopies = false;

		const auto mousePos = ImGui::GetMousePos();

		ImVec2 avgPosition = ImVec2{ 0.f, 0.f };
		for (const auto& n : myNodeCopies)
		{
			avgPosition += ed::GetNodePosition(ed::NodeId{ n->id });
		}

		avgPosition /= (float)myNodeCopies.size();

		ed::ClearSelection();

		for (const auto& n : myNodeCopies)
		{
			const ImVec2 newPosition = ed::GetNodePosition(ed::NodeId{ n->id }) - avgPosition + mousePos;
			ed::SetNodePosition(ed::NodeId{ n->id }, newPosition);
			ed::SelectNode(ed::NodeId{ n->id }, true);
		}

		myNodeCopies.clear();
	}
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawPanels()

{
	DrawNodesPanel();
	DrawGraphDataPanel();
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::DrawContextPopups()
{
	DrawNodeContextMenu();
	DrawPinContextMenu();
	DrawLinkContextMenu();
	DrawBackgroundContextMenu();
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline const IONodeGraphEditor<graphType, EditorBackend>::IncompatiblePinReason IONodeGraphEditor<graphType, EditorBackend>::CanLinkPins(const UUID64 input, const UUID64 output)
{
	auto* startAttr = myOpenGraph->GetAttributeByID(input);
	auto* endAttr = myOpenGraph->GetAttributeByID(output);

	if (startAttr && endAttr)
	{
		if (startAttr == endAttr)
		{
			return IncompatiblePinReason::SamePin;
		}

		if (startAttr->direction == GraphKey::AttributeDirection::Input)
		{
			std::swap(startAttr, endAttr);
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

		if (!sameType)
		{
			return IncompatiblePinReason::IncompatibleType;
		}

		if (!startAttr->linkable || !endAttr->linkable)
		{
			return IncompatiblePinReason::NonLinkable;
		}

		if (startAttr->direction == endAttr->direction)
		{
			return IncompatiblePinReason::IncompatibleDirection;
		}
	}

	return IncompatiblePinReason::None;
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::AddNode(Ref<GraphKey::Node> node)
{
	Vector<UUID64> pins;
	for (auto& i : node->inputs)
	{
		pins.emplace_back(i.id);
	}

	for (auto& o : node->outputs)
	{
		pins.emplace_back(o.id);
	}

	Editor::CreateNode(node->id, pins);
	myOpenGraph->AddNode(node);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::CreateLink(const UUID64 id, const UUID64 input, const UUID64 output)
{
	Editor::CreateLink(id, input, output);
	myOpenGraph->CreateLink(id, input, output);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::AddLink(GraphKey::Link link)
{
	Editor::CreateLink(link.id, link.input, link.output);
	myOpenGraph->AddLink(link);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::RemoveLink(const UUID64 linkId)
{
	Editor::RemoveLink(linkId);
	myOpenGraph->RemoveLink(linkId);

}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::RemoveNode(const UUID64 nodeId)
{
	Editor::RemoveNode(nodeId);
	myOpenGraph->RemoveNode(nodeId);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::ReconstructGraph()
{
	auto& backend = GetBackend();
	backend.links.clear();
	backend.nodes.clear();

	for (const auto& l : myOpenGraph->GetLinks())
	{
		NodeGraph::Link	newLink{};
		newLink.id = l.id;
		newLink.input = l.input;
		newLink.output = l.output;

		backend.links.emplace_back(newLink);
	}

	for (const auto& n : myOpenGraph->GetNodes())
	{
		auto& newNode = backend.nodes.emplace_back();
		newNode.id = n->id;

		for (const auto& i : n->inputs)
		{
			newNode.pins.emplace_back(i.id);
		}

		for (const auto& o : n->outputs)
		{
			newNode.pins.emplace_back(o.id);
		}
	}

	InitializeEditor(nullptr);
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawNodesPanel()
{
	ImGui::SetNextWindowClass(GetWindowClass());

	const std::string id = "Nodes##" + myContext;
	if (ImGui::Begin(id.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());
		DrawNodeList(mySearchQuery);
	}
	ImGui::End();
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::OnBeginCreate()
{
	auto showLabel = [](const char* label, ImColor color)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
		const auto size = ImGui::CalcTextSize(label);

		const auto padding = ImGui::GetStyle().FramePadding;
		const auto spacing = ImGui::GetStyle().ItemSpacing;

		ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2{ spacing.x, -spacing.y });

		const auto rectMin = ImGui::GetCursorScreenPos() - padding;
		const auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

		auto drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
		ImGui::TextUnformatted(label);
	};

	ed::PinId startPinId = 0, endPinId = 0;
	if (ed::QueryNewLink(&startPinId, &endPinId))
	{
		auto* startAttr = myOpenGraph->GetAttributeByID(startPinId.Get());
		auto* endAttr = myOpenGraph->GetAttributeByID(endPinId.Get());

		myNewLinkPinId = startAttr ? startAttr->id : endAttr->id;

		const auto reason = CanLinkPins(endPinId.Get(), startPinId.Get());
		if (reason == IncompatiblePinReason::SamePin)
		{
			ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
		}
		else if (reason == IncompatiblePinReason::IncompatibleType)
		{
			showLabel("x Incompatible Type", ImColor{ 255, 0, 0 });
			ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
		}
		else if (reason == IncompatiblePinReason::IncompatibleDirection)
		{
			showLabel("x Incompatible Direction", ImColor{ 255, 0, 0 });
			ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
		}
		else if (reason == IncompatiblePinReason::NonLinkable)
		{
			showLabel("x Pin is not linkable", ImColor{ 255, 0, 0 });
			ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
		}
		else if (ed::AcceptNewItem(ImColor{ 1.f, 1.f, 1.f }, 2.f))
		{
			auto inputDirAttr = startAttr->direction == GraphKey::AttributeDirection::Input ? startAttr : endAttr;
			Vector<UUID64> linksToRemove{};
			if (!inputDirAttr->links.empty())
			{
				linksToRemove.insert(linksToRemove.end(), inputDirAttr->links.begin(), inputDirAttr->links.end());
			}

			for (const auto& l : linksToRemove)
			{
				RemoveLink(l);
			}

			//if (myGraphType == GraphKey::GraphType::Animation && startAttr->type == GraphKey::AttributeType::AnimationPose)
			//{
			//	Vector<UUID64> linksToRemove{};

			//	if (!startAttr->links.empty())
			//	{
			//		linksToRemove.insert(linksToRemove.end(), startAttr->links.begin(), startAttr->links.end());
			//	}

			//	if (!endAttr->links.empty())
			//	{
			//		linksToRemove.insert(linksToRemove.end(), endAttr->links.begin(), endAttr->links.end());
			//	}

			//	for (const auto& l : linksToRemove)
			//	{
			//		RemoveLink(l);
			//	}
			//}

			myCommandStack->AddCommand<NodeGraphGraphKeyCommand>(myOpenGraph);
			const UUID64 id{};
			CreateLink(id, startPinId.Get(), endPinId.Get());
		}
	}

	ed::PinId pinId = 0;
	if (ed::QueryNewNode(&pinId))
	{
		myNewLinkPinId = pinId.Get();
		auto newLinkPin = myOpenGraph->GetAttributeByID(pinId.Get());
		if (newLinkPin)
		{
			showLabel("+ Create Node", ImColor(32, 45, 32, 180));
		}

		const glm::vec4 draggedLinkColor = newLinkPin ? GetColorFromAttribute(*newLinkPin) : glm::vec4(1.f, 1.f, 1.f, 1.f);
		const float lineThickness = 2.f;

		if (ed::AcceptNewItem(ImColor{ draggedLinkColor.x, draggedLinkColor.y, draggedLinkColor.z }, lineThickness))
		{
			myCreateNewNode = true;
			myNewNodeLinkPinId = pinId.Get();
			myNewLinkPinId = 0;

			ed::Suspend();
			UI::OpenPopup("BackgroundContextMenu");
			ed::Resume();
		}
	}
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::OnDeleteLink(const UUID64 id)
{
	myOpenGraph->RemoveLink(id);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::OnDeleteNode(const UUID64 id)
{
	myOpenGraph->RemoveNode(id);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::OnCopy()
{
	const auto selectedNodes = GetSelectedNodes();
	const auto selectedLinks = GetSelectedLinks();

	myLinkCopies.clear();
	myNodeCopies.clear();

	for (const auto& id : selectedNodes)
	{
		auto node = myOpenGraph->GetNodeByID(id);
		if (node)
		{
			auto newNode = node->CreateCopy(myOpenGraph.get());
			myNodeCopies.emplace_back(newNode);
		}
	}

	for (const auto& id : selectedLinks)
	{
		auto* link = myOpenGraph->GetLinkByID(id);
		if (link)
		{
			myLinkCopies.emplace_back(*link);
		}
	}
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::OnPaste()
{
	struct Link
	{
		UUID64 input = 0;
		UUID64 output = 0;

		UUID64 id{};
	};

	Vector<std::pair<std::pair<GraphKey::Attribute*, GraphKey::Attribute*>, GraphKey::Link>> linksToCopy;

	for (const auto& n : myNodeCopies)
	{
		for (const auto& i : n->inputs)
		{
			for (const auto& l : i.links)
			{
				auto itFirst = std::find_if(myNodeCopies.begin(), myNodeCopies.end(), [&](const auto& lhs)
				{
					auto* link = myOpenGraph->GetLinkByID(l);
					auto inputNode = myOpenGraph->GetNodeFromAttributeID(link->input);

					return (inputNode->id == lhs->id);
				});

				auto itSecond = std::find_if(myNodeCopies.begin(), myNodeCopies.end(), [&](const auto& lhs)
				{
					auto* link = myOpenGraph->GetLinkByID(l);
					auto outputNode = myOpenGraph->GetNodeFromAttributeID(link->output);

					return (outputNode->id == lhs->id);
				});

				if (itFirst != myNodeCopies.end() && itSecond != myNodeCopies.end())
				{
					auto linkIt = std::find_if(linksToCopy.begin(), linksToCopy.end(), [&](const auto& lhs)
					{
						return l == lhs.second.id;
					});

					if (linkIt == linksToCopy.end())
					{
						auto* link = myOpenGraph->GetLinkByID(l);

						GraphKey::Attribute* inputAttribute = nullptr;
						GraphKey::Attribute* outputAttribute = nullptr;

						for (auto& input : (*itFirst)->inputs)
						{
							if (input.id == link->input)
							{
								inputAttribute = &input;
								break;
							}
						}

						for (auto& output : (*itSecond)->outputs)
						{
							if (output.id == link->output)
							{
								outputAttribute = &output;
								break;
							}
						}

						linksToCopy.emplace_back(std::pair(std::pair{ inputAttribute, outputAttribute }, * link));
					}
				}
			}
		}
	}

	for (auto& node : myNodeCopies)
	{
		for (auto& input : node->inputs)
		{
			input.id = {};
			input.links.clear();
		}

		for (auto& output : node->outputs)
		{
			output.id = {};
			output.links.clear();
		}
	}

	Vector<GraphKey::Link> newLinks{};
	for (auto& [attributes, link] : linksToCopy)
	{
		auto& input = attributes.first;
		auto& output = attributes.second;

		auto& newLink = newLinks.emplace_back();
		newLink.input = input->id;
		newLink.output = output->id;

		input->links.emplace_back(newLink.id);
		output->links.emplace_back(newLink.id);
	}

	myCommandStack->AddCommand<NodeGraphGraphKeyCommand>(myOpenGraph);

	for (const auto& n : myNodeCopies)
	{
		n->id = {};
		AddNode(n);
	}

	for (const auto& l : newLinks)
	{
		AddLink(l);
	}

	myShouldMoveCopies = true;
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::DrawGraphDataPanel()
{
	ImGui::SetNextWindowClass(GetWindowClass());

	const std::string id = "Graph##" + myContext;
	if (ImGui::Begin(id.c_str(), nullptr, ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		UI::Header("Graph");

		ImGui::Separator();

		if (myOpenGraph)
		{
			UI::ScopedStyleFloat rounding(ImGuiStyleVar_FrameRounding, 2.f);

			if (ImGui::CollapsingHeader("Variables"))
			{
				{
					UI::ScopedButtonColor buttCol{ EditorTheme::Buttons::AddButton };
					ImGui::Button("Add +##param");
				}

				if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
				{
					for (const auto& [typeName, typeInfo] : GraphKey::TypeRegistry::GetTypeRegistry())
					{
						if (ImGui::MenuItem(typeName.c_str()))
						{
							myOpenGraph->AddParameter("New " + typeName, typeInfo.defaultValue);
						}
					}
					ImGui::EndPopup();
				}

				const auto flags = ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable;
				if (ImGui::BeginTable("##parameterTable", 2, flags, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2.f }))
				{
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);

					ImGui::TableHeadersRow();

					for (auto& param : myOpenGraph->GetBlackboard())
					{
						ImGui::TableNextColumn();
						const std::string strId = "##" + std::to_string(UI::GetID());
						ImGui::PushItemWidth(ImGui::GetColumnWidth());
						ImGui::InputTextString(strId.c_str(), &param.name);
						ImGui::PopItemWidth();

						ImGui::TableNextColumn();

						const auto type = GraphKey::TypeRegistry::GetNameFromTypeIndex(param.value.type());
						ImGui::TextUnformatted(type.c_str());

						ImGui::SameLine();

						std::string buttonId = "-##" + param.name;
						if (ImGui::Button(buttonId.c_str(), { 22.f, 22.f }))
						{
							myOpenGraph->RemoveParameter(param.name);
							break;
						}
					}
					ImGui::EndTable();
				}
			}

			if (ImGui::CollapsingHeader("Events"))
			{
				{
					UI::ScopedButtonColor buttCol{ EditorTheme::Buttons::AddButton };
					if (ImGui::Button("Add +##event"))
					{
						myOpenGraph->AddEvent("New Event");
					}
				}

				for (auto& e : myOpenGraph->GetEvents())
				{
					const std::string strId = "##" + std::to_string(UI::GetID());
					ImGui::InputTextString(strId.c_str(), &e.name);
				}
			}
		}
	}

	ImGui::End();
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline Ref<GraphKey::Node> IONodeGraphEditor<graphType, EditorBackend>::DrawNodeList(std::string& query, std::type_index typeIndex)
{
	std::map<std::string, Vector<std::string>> nodeCategories;
	Vector<std::string> paramNodeNames{};
	Vector<std::string> eventNodeNames{};

	Ref<GraphKey::Node> node;

	if (myOpenGraph)
	{
		for (const auto& currNode : GraphKey::Registry::GetNodesOfGraphType(myGraphType))
		{
			if (currNode.visible)
			{
				nodeCategories[currNode.category].emplace_back(currNode.name);
			}
		}

		for (const auto& param : myOpenGraph->GetBlackboard())
		{
			nodeCategories["Variables"].emplace_back("Get " + param.name);
			nodeCategories["Variables"].emplace_back("Set " + param.name);

			paramNodeNames.emplace_back("Get " + param.name);
			paramNodeNames.emplace_back("Set " + param.name);
		}

		for (const auto& e : myOpenGraph->GetEvents())
		{
			nodeCategories["Events"].emplace_back("On " + e.name);
			eventNodeNames.emplace_back("On " + e.name);
		}
	}

	if (typeIndex == std::type_index(typeid(Volt::Entity)))
	{
		const auto eventNames = GetEventNamesFromCurrentNewPin();
		for (const auto& e : eventNames)
		{
			nodeCategories["Events"].emplace_back("Call " + e);
			eventNodeNames.emplace_back("Call " + e);
		}
	}

	UI::PushID();

	ImGui::BeginChild("Main", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		bool hasQuery;

		EditorUtils::SearchBar(query, hasQuery, myActivateSearchWidget);
		if (myActivateSearchWidget)
		{
			myActivateSearchWidget = false;
		}

		UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
		ImGui::BeginChild("Scrollable", ImGui::GetContentRegionAvail());
		{
			for (const auto& [category, names] : nodeCategories)
			{
				auto nodeInCategoryVisible = [&]()
				{
					if (query.empty())
					{
						return true;
					}

					bool visible = false;
					const std::string lowerQuery = Utility::ToLower(query);

					for (const auto& n : names)
					{
						const std::string lowerName = Utility::ToLower(n);
						visible |= Utility::StringContains(lowerName, lowerQuery);
					}

					const std::string lowerCategory = Utility::ToLower(category);
					visible |= Utility::StringContains(lowerCategory, lowerQuery);

					return visible;
				};

				auto nodeHasPinOfType = [&](const std::string& name)
				{
					if (typeIndex == typeid(void))
					{
						return true;
					}

					Ref<GraphKey::Node> n;

					if (category == "Variables")
					{
						n = Utility::CreateNodeFromParameterName(name, myOpenGraph);
					}
					else if (category == "Events")
					{
						const std::string pType = name.substr(0, name.find_first_of(' '));

						if (pType == "On")
						{
							n = Utility::CreateRecieveNodeFromEventName(name, myOpenGraph, myCurrentScene);
						}
						else
						{
							n = Utility::CreateCallNodeFromEventName(name, GetEntityFromCurrentNewPin());
						}
					}
					else
					{
						n = GraphKey::Registry::Create(name);
					}

					for (const auto& input : n->inputs)
					{
						if (std::type_index{ input.data.type() } == typeIndex)
						{
							return true;
						}
					}

					for (const auto& output : n->outputs)
					{
						if (std::type_index{ output.data.type() } == typeIndex)
						{
							return true;
						}
					}

					return false;
				};

				const bool nodeInCategory = nodeInCategoryVisible();

				if (!nodeInCategory)
				{
					continue;
				}

				bool categoryHasNodeOfPin = false;
				for (const auto& name : names)
				{
					categoryHasNodeOfPin |= nodeHasPinOfType(name);
				}

				if (!categoryHasNodeOfPin)
				{
					continue;
				}

				if (nodeInCategory && !query.empty())
				{
					ImGui::SetNextItemOpen(true);
				}

				if (nodeInCategory && ImGui::TreeNodeEx(category.c_str()))
				{
					Vector<std::string> tempNames = names;
					if (!query.empty())
					{
						tempNames = UI::GetEntriesMatchingQuery(query, names);
					}

					for (const auto& name : tempNames)
					{
						const bool nodeHasPin = nodeHasPinOfType(name);
						const bool containsQuery = Utility::StringContains(Utility::ToLower(name), Utility::ToLower(query));

						UI::RenderMatchingTextBackground(query, name, EditorTheme::MatchingTextBackground);
						if (containsQuery && nodeHasPin && ImGui::MenuItem(name.c_str()) && myOpenGraph)
						{
							if (category == "Variables")
							{
								node = Utility::CreateNodeFromParameterName(name, myOpenGraph);
							}
							else if (category == "Events")
							{
								const std::string pType = name.substr(0, name.find_first_of(' '));

								if (pType == "On")
								{
									node = Utility::CreateRecieveNodeFromEventName(name, myOpenGraph, myCurrentScene);
								}
								else
								{
									node = Utility::CreateCallNodeFromEventName(name, GetEntityFromCurrentNewPin());
								}
							}
							else
							{
								node = GraphKey::Registry::Create(name);
							}

							myCommandStack->AddCommand<NodeGraphGraphKeyCommand>(myOpenGraph);
							AddNode(node);
						}
					}

					ImGui::TreePop();
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	UI::PopID();

	return node;
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawNodeContextMenu()
{
	if (UI::BeginPopup("NodeContextMenu"))
	{
		ImGui::MenuItem("Delete");

		UI::EndPopup();
	}
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawPinContextMenu()
{
	if (UI::BeginPopup("PinContextMenu"))
	{
		GraphKey::Attribute* pin = myOpenGraph->GetAttributeByID(GetContext().contextPinId.Get());
		if (pin->data.has_value())
		{
			const bool isEntity = pin->data.type() == GetTypeIndex<Volt::Entity>();
			if (isEntity)
			{
				if (SelectionManager::IsAnySelected() && ImGui::MenuItem("Assign Selected Entity"))
				{
					pin->data = myCurrentScene->GetEntityFromUUID(SelectionManager::GetSelectedEntities().at(0));
				}

				if (ImGui::MenuItem("Assign Graph Entity"))
				{
					pin->data = myCurrentScene->GetEntityFromUUID(myOpenGraph->GetEntity());
				}
			}
		}

		UI::EndPopup();
	}
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawLinkContextMenu()
{
	if (UI::BeginPopup("LinkContextMenu"))
	{
		ImGui::MenuItem("Delete");

		UI::EndPopup();
	}
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawBackgroundContextMenu()
{
	if (!myOpenGraph)
	{
		return;
	}

	ImGui::SetNextWindowSize({ 250.f, 400.f });
	static ImVec2 newNodePostion{ 0.0f, 0.0f };

	if (UI::BeginPopup("BackgroundContextMenu"))
	{
		std::type_index idx = std::type_index{ typeid(void) };
		if (auto* startPin = myOpenGraph->GetAttributeByID(myNewNodeLinkPinId))
		{
			idx = startPin->data.type();
		}

		Ref<GraphKey::Node> node = DrawNodeList(myContextSearchQuery, idx);
		newNodePostion = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup());

		if (node)
		{
			myCreateNewNode = false;
			ed::SetNodePosition(ed::NodeId{ node->id }, newNodePostion);

			if (auto* startedAtPin = myOpenGraph->GetAttributeByID(myNewNodeLinkPinId))
			{
				auto& pins = startedAtPin->direction == GraphKey::AttributeDirection::Input ? node->outputs : node->inputs;
				VT_ASSERT(!pins.empty());

				for (auto& pin : pins)
				{
					ed::PinId startId = { startedAtPin->id };
					ed::PinId endId = { pin.id };

					const auto reason = CanLinkPins(startId.Get(), endId.Get());

					if (reason == IncompatiblePinReason::None)
					{
						auto* startPin = myOpenGraph->GetAttributeByID(startId.Get());
						auto* endPin = myOpenGraph->GetAttributeByID(endId.Get());

						if (myGraphType == GraphKey::GraphType::Animation && startPin->type == GraphKey::AttributeType::Flow)
						{
							Vector<UUID64> linksToRemove{};

							if (!startPin->links.empty())
							{
								linksToRemove.insert(linksToRemove.end(), startPin->links.begin(), startPin->links.end());
							}

							if (!endPin->links.empty())
							{
								linksToRemove.insert(linksToRemove.end(), endPin->links.begin(), endPin->links.end());
							}

							for (const auto& l : linksToRemove)
							{
								RemoveLink(l);
							}
						}
						else
						{
							if (!startPin->links.empty())
							{
								for (const auto& l : GetBackend().links)
								{
									if (l.input == endId.Get())
									{
										RemoveLink(l.id);
									}

								}

								startPin->links.clear();
							}
						}

						if (startPin->direction == GraphKey::AttributeDirection::Input)
						{
							std::swap(startId, endId);
						}

						myCommandStack->AddCommand<NodeGraphGraphKeyCommand>(myOpenGraph);

						const UUID64 id{};
						CreateLink(id, startId.Get(), endId.Get());

						break;
					}
				}

			}

			myNewNodeLinkPinId = 0;
			ImGui::CloseCurrentPopup();
		}
		UI::EndPopup();
	}
	else
	{
		myCreateNewNode = false;
		myContextSearchQuery.clear();
		myNewNodeLinkPinId = 0;
	}
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline void IONodeGraphEditor<TGraphType, EditorBackend>::InitializeStyle(ed::Style& editorStyle)
{
	// Style
	editorStyle.NodePadding = { 0.0f, 4.0f, 0.0f, 0.0f }; // This mostly affects Comment nodes
	editorStyle.NodeRounding = 3.0f;
	editorStyle.NodeBorderWidth = 1.0f;
	editorStyle.HoveredNodeBorderWidth = 2.0f;
	editorStyle.SelectedNodeBorderWidth = 3.0f;
	editorStyle.PinRounding = 2.0f;
	editorStyle.PinBorderWidth = 0.0f;
	editorStyle.LinkStrength = 80.0f;
	editorStyle.ScrollDuration = 0.35f;
	editorStyle.FlowMarkerDistance = 30.0f;
	editorStyle.FlowDuration = 2.0f;
	editorStyle.GroupRounding = 0.0f;
	editorStyle.GroupBorderWidth = 0.0f;

	// Extra (for now just using defaults)
	editorStyle.PinCorners = ImDrawFlags_RoundCornersAll;

	// Colors
	editorStyle.Colors[ed::StyleColor_Bg] = ImColor(23, 24, 28, 200);
	editorStyle.Colors[ed::StyleColor_Grid] = ImColor(21, 21, 21, 255);// ImColor(60, 60, 60, 40);
	editorStyle.Colors[ed::StyleColor_NodeBg] = ImColor(31, 33, 38, 255);
	editorStyle.Colors[ed::StyleColor_NodeBorder] = ImColor(51, 54, 62, 140);
	editorStyle.Colors[ed::StyleColor_HovNodeBorder] = ImColor(60, 180, 255, 150);
	editorStyle.Colors[ed::StyleColor_SelNodeBorder] = ImColor(255, 225, 135, 255);
	editorStyle.Colors[ed::StyleColor_NodeSelRect] = ImColor(5, 130, 255, 64);
	editorStyle.Colors[ed::StyleColor_NodeSelRectBorder] = ImColor(5, 130, 255, 128);
	editorStyle.Colors[ed::StyleColor_HovLinkBorder] = ImColor(60, 180, 255, 255);
	editorStyle.Colors[ed::StyleColor_SelLinkBorder] = ImColor(255, 225, 135, 255);
	editorStyle.Colors[ed::StyleColor_LinkSelRect] = ImColor(5, 130, 255, 64);
	editorStyle.Colors[ed::StyleColor_LinkSelRectBorder] = ImColor(5, 130, 255, 128);
	editorStyle.Colors[ed::StyleColor_PinRect] = ImColor(60, 180, 255, 255);
	editorStyle.Colors[ed::StyleColor_PinRectBorder] = ImColor(60, 180, 255, 255);
	editorStyle.Colors[ed::StyleColor_Flow] = ImColor(255, 128, 64, 255);
	editorStyle.Colors[ed::StyleColor_FlowMarker] = ImColor(255, 128, 64, 255);
	editorStyle.Colors[ed::StyleColor_GroupBg] = ImColor(255, 255, 255, 30);
	editorStyle.Colors[ed::StyleColor_GroupBorder] = ImColor(0, 0, 0, 0);

	//editorStyle.Colors[ed::StyleColor_HighlightLinkBorder] = ImColor(255, 255, 255, 140);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline const Volt::Entity IONodeGraphEditor<TGraphType, EditorBackend>::GetEntityFromCurrentNewPin()
{
	if (myNewNodeLinkPinId == UUID64(0))
	{
		return {};
	}

	auto pin = myOpenGraph->GetAttributeByID(myNewNodeLinkPinId);
	if (!pin)
	{
		return {};
	}

	if (pin->data.type() != typeid(Volt::Entity))
	{
		return {};
	}

	return std::any_cast<Volt::Entity>(pin->data);
}

template<GraphKey::GraphType TGraphType, typename EditorBackend>
inline const Vector<std::string> IONodeGraphEditor<TGraphType, EditorBackend>::GetEventNamesFromCurrentNewPin()
{
	auto entity = GetEntityFromCurrentNewPin();
	if (!entity)
	{
		return {};
	}

	//if (!entity.HasComponent<Volt::VisualScriptingComponent>())
	//{
	//	return{};
	//}

	//auto& comp = entity.GetComponent<Volt::VisualScriptingComponent>();
	//if (!comp.graph)
	//{
	//	return {};
	//}

	//const auto& events = comp.graph->GetEvents();
	//Vector<std::string> result{};
	//for (const auto& e : events)
	//{
	//	result.emplace_back(e.name);
	//}

	//return result;

	return {};
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline const glm::vec4 IONodeGraphEditor<graphType, EditorBackend>::GetColorFromAttribute(const GraphKey::Attribute& attr)
{
	const auto typeIndex = std::type_index(attr.data.type());
	if (attr.type == GraphKey::AttributeType::Flow)
	{
		return { 1.f, 1.f, 1.f, 1.f };
	}
	else
	{
		const auto& attribColors = IONodeGraphEditorHelpers::GetAttribColors();

		if (attribColors.contains(typeIndex))
		{
			return attribColors.at(typeIndex);
		}
	}

	return IONodeGraphEditorHelpers::GetDefaultPinColor();
}

template<GraphKey::GraphType graphType, typename EditorBackend>
inline void IONodeGraphEditor<graphType, EditorBackend>::DrawPinIcon(const GraphKey::Attribute& pin, bool connected, ImColor color, int32_t alpha)
{
	IconType iconType;
	switch (pin.type)
	{
		case GraphKey::AttributeType::Flow: iconType = IconType::Flow;   break;
		case GraphKey::AttributeType::Type: iconType = IconType::Circle;   break;
		case GraphKey::AttributeType::AnimationPose: iconType = IconType::AnimationPose;   break;

		default:
			return;
	}

	constexpr int32_t iconSize = 24;
	const ImVec2 size = ImVec2(iconSize, iconSize);

	if (ImGui::IsRectVisible(size))
	{
		auto cursorPos = ImGui::GetCursorScreenPos();
		auto drawList = ImGui::GetWindowDrawList();

		DrawIconKey(drawList, cursorPos, cursorPos + size, iconType, connected, color, ImColor(32, 32, 32, alpha));
	}

	ImGui::Dummy(size);
}
