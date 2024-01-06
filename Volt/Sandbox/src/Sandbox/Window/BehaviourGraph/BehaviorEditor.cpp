#include "sbpch.h"
#include "BehaviorEditor.h"
#include "BehaviorPanel.h"
#include <Volt/Asset/Importers/BehaviorTreeImporter.h>
#include <builders.h>
#include <typeindex>
#include <Volt/Utility/UIUtility.h>
#include <filesystem>
#include "Sandbox/Utility/EditorUtilities.h"
#include <Volt/Project/ProjectManager.h>

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

static inline ImRect ImGui_GetItemRect()
{
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

void BehaviorEditor::SortOutput()
{
	for (auto& _node : myBehaviourTree->GetNodeManager().GetLinksNoConst())
	{
		std::sort(_node.second.begin(), _node.second.end(), [&](Volt::BehaviorTree::Link a1, Volt::BehaviorTree::Link a2) {
			auto a1p = ed::GetNodePosition(ed::NodeId(a1.m_childID));
			auto a2p = ed::GetNodePosition(ed::NodeId(a2.m_childID));
			return a1p.x < a2p.x;
		});
	}
}

bool BehaviorEditor::CanLink(OwnedPin in_pin1, OwnedPin in_pin2)
{
	if (in_pin1.type == in_pin2.type)
		return false;
	if (in_pin2.type == ePinType::OUTPUT)
		std::swap(in_pin1, in_pin2);
	bool checkNodeOutputAvailability = true;
	auto nPtr = myBehaviourTree->GetNodeManager().GetNodeFromUUID(in_pin1.nodeUUID);
	auto outputNodeKind = nPtr->m_kind;
	if (outputNodeKind != Volt::BehaviorTree::eNodeKind::ROOT && outputNodeKind != Volt::BehaviorTree::eNodeKind::DECORATOR)
		checkNodeOutputAvailability = false;

	for (auto& lnk : GetBackend().links)
	{
		if (lnk.output == in_pin1.pinUUID && lnk.input == in_pin2.pinUUID)
			return false;
		if (lnk.input == in_pin1.pinUUID || lnk.input == in_pin2.pinUUID)
			return false;

		if (checkNodeOutputAvailability)
			if (in_pin1.pinUUID == lnk.output)
				return false;
	}
	return true;
}

BehaviorEditor::BehaviorEditor(const std::string& title, const std::string& context)
	: NodeGraph::Editor(title, context, true, CreateRef<BehaviorGraphBackend>())
{
	{
		myBehaviourTree = CreateRef<Volt::BehaviorTree::Tree>();
		Ref<NodeGraph::EditorContext> editorContext = CreateRef<NodeGraph::EditorContext>();
		m_scene = Volt::Scene::CreateDefaultScene("Particle Editor", false);

		// #TODO_Ivar: What??
		/*myTreeEntity = Volt::Entity(56, m_scene.get());
		myTreeEntity.AddComponent <Volt::BehaviorTreeComponent>();*/

		editorContext->onBeginCreate = [&]()
		{
			static Volt::UUID newNodeLinkPin = 0;
			static Volt::UUID newLinkPin = 0;
			static bool createNewNode = false;

			auto showLabel = [](const char* label, ImColor color)
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
				auto size = ImGui::CalcTextSize(label);

				auto padding = ImGui::GetStyle().FramePadding;
				auto spacing = ImGui::GetStyle().ItemSpacing;

				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

				auto rectMin = ImGui::GetCursorScreenPos() - padding;
				auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

				auto drawList = ImGui::GetWindowDrawList();
				drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
				ImGui::TextUnformatted(label);
			};

			ed::PinId startPinId = 0, endPinId = 0;
			if (ed::QueryNewLink(&startPinId, &endPinId))
			{
				auto startPin = FindPin(startPinId);
				auto endPin = FindPin(endPinId);

				newLinkPin = startPin.pinUUID ? startPin.pinUUID : endPin.pinUUID;

				if (startPin.type == ePinType::INPUT)
				{
					std::swap(startPin, endPin);
					std::swap(startPinId, endPinId);
				}

				if (startPin.pinUUID != 0 && endPin.pinUUID != 0)
				{
					if (endPin.pinUUID == startPin.pinUUID)
					{
						ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
					}
					else if (!CanLink(startPin, endPin))
					{
						showLabel("x Incompatible Pin", ImColor(45, 32, 32, 180));
						ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
					}
					else
					{
						showLabel("+ Create Link", ImColor(32, 45, 32, 180));
						if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
						{
							GetBackend().CreateLink(startPin.pinUUID, endPin.pinUUID);
							myBehaviourTree->GetNodeManager().RegisterLink(startPin.nodeUUID, endPin.nodeUUID, GetBackend().links.back().id);
							SortOutput();
						}
					}
				}
			}
		};

		editorContext->onDeleteLink = [this](const Volt::UUID id)
		{
			myBehaviourTree->GetNodeManager().UnregisterLink(id);
			SortOutput();
		};

		editorContext->onDeleteNode = [this](const Volt::UUID id)
		{
			if (id != myBehaviourTree->GetRoot())
			{
				myBehaviourTree->GetNodeManager().UnregisterNode(id);
				SortOutput();
			}
			else
			{
				auto rootNode = myBehaviourTree->GetRoot();
				myBehaviourTree->ClearRootLink();
				GetBackend().nodes.push_back(NodeGraph::Node(rootNode));
				GetBackend().nodes.back().pins.push_back(Volt::UUID());
				GetBackend().nodes.back().pins.push_back(Volt::UUID());

				SelectNode(ed::NodeId(rootNode));
			}
		};

		editorContext->onShowBackgroundContextMenu = [this]()
		{
			UI::OpenPopup("New Node");
		};

		InitializeEditor(editorContext);

		GetBackend().nodes.push_back(NodeGraph::Node(myBehaviourTree->GetRoot()));
		GetBackend().nodes.back().pins.push_back(Volt::UUID());
		GetBackend().nodes.back().pins.push_back(Volt::UUID());
	}

}


BehaviorEditor::~BehaviorEditor()
{
}

void BehaviorEditor::OpenAsset(Ref<Volt::Asset> asset)
{
	m_currentHandle = asset->handle;
	auto importedTree = Volt::AssetManager::GetAsset<Volt::BehaviorTree::Tree>(m_currentHandle);
	if (!importedTree || !importedTree->IsValid())
		return;
	GetBackend().links.clear();
	GetBackend().nodes.clear();
	for (const auto& _nodePair : importedTree->GetNodeManager().GetNodes())
	{
		AddNodeToBackend(_nodePair.first);
	}
	for (const auto& _linkPair : importedTree->GetNodeManager().GetLinkIDs())
	{
		NodeGraph::Link ngLink;
		ngLink.id = _linkPair.first;
		for (const auto& node : GetBackend().nodes)
		{
			if (_linkPair.second.m_parentID == node.id)
			{
				ngLink.output = node.pins[1];
			}
			if (_linkPair.second.m_childID == node.id)
			{
				ngLink.input = node.pins[0];
			}
		}
		GetBackend().links.emplace_back(ngLink);
	}
	InitializeEditor(nullptr);
	myBehaviourTree = importedTree;

}

void BehaviorEditor::AddNodeToBackend(const Volt::UUID& in_nodeID)
{
	GetBackend().nodes.push_back(NodeGraph::Node(in_nodeID));
	GetBackend().nodes.back().pins.push_back(Volt::UUID());
	GetBackend().nodes.back().pins.push_back(Volt::UUID());
}

void BehaviorEditor::DrawNodes()
{
	for (const auto& n : GetBackend().nodes)
	{
		NodeDraw(n);
	}
	for (const auto& lnk : GetBackend().links)
	{
		ed::Link(ed::LinkId(lnk.id), ed::PinId(lnk.output), ed::PinId(lnk.input), { 1,1,1,1 }, 2.0f);
	}
}
void BehaviorEditor::NodeDraw(const NodeGraph::Node& n)
{
	const float rounding = 2.0f;
	const float padding = 12.0f;

	const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_Bg];

	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 255));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 255));
	ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 255));
	ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 255));

	ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
	ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
	ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
	ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
	ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
	ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
	ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
	ed::BeginNode(ed::NodeId(n.id));

	ImGui::BeginVertical(ed::NodeId(n.id).AsPointer());
	ImGui::BeginHorizontal("inputs");
	ImGui::Spring(0, padding * 2);

	ImRect inputsRect;
	int inputAlpha = 175;
	if (n.id != myBehaviourTree->GetRoot())
	{
		auto& pin = n.pins[0];

		ImGui::Dummy(ImVec2(1, padding));
		ImGui::Spring(1, 0);
		inputsRect = ImGui_GetItemRect();
		ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
		ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);

#if IMGUI_VERSION_NUM > 18101
		ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);
#else
		ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
#endif
		ed::BeginPin(ed::PinId(pin), ed::PinKind::Input);
		ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
		ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
		ed::EndPin();
		ed::PopStyleVar(3);

		/*if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
			inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));*/

	}
	else ImGui::Dummy(ImVec2(0, padding));

	ImGui::Spring(0, padding * 2);
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("content_frame");
	ImGui::Spring(1, padding);

	ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
	ImGui::Dummy(ImVec2(108, 0));
	ImGui::Spring(1);
	ImGui::TextUnformatted(myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id)->m_name.c_str());
	if (myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id)->m_kind == Volt::BehaviorTree::eNodeKind::LEAF)
	{
		auto lPtr = reinterpret_cast<Volt::BehaviorTree::Leaf*>(myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id).get());
		ImGui::Spring();
		auto sizeOfField = ImGui::CalcTextSize(lPtr->m_monoScriptFunctonName.c_str()).x + 20;
		if (sizeOfField < 100)
			sizeOfField = 100;
		ImGui::PushItemWidth(sizeOfField);
		if (ImGui::InputTextString("",
			&lPtr->m_monoScriptFunctonName))
		{
		}
		ImGui::PopItemWidth();
	}
	if (myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id)->m_kind == Volt::BehaviorTree::eNodeKind::DECORATOR)
	{
		auto dPtr = reinterpret_cast<Volt::BehaviorTree::Decorator*>(myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id).get());
		ImGui::Spring();
		if (ImGui::Button(Volt::BehaviorTree::decBfr[(int)dPtr->m_type].c_str()))
		{
			ed::Suspend();
			UI::OpenPopup("Decorator thing");
			ed::Resume();
			m_decPtr = dPtr;
		}
		if (dPtr->m_type == Volt::BehaviorTree::eDecoratorType::IF)
		{
			auto sizeOfField = ImGui::CalcTextSize(dPtr->m_if.c_str()).x + 20;
			if (sizeOfField < 100)
				sizeOfField = 100;
			ImGui::PushItemWidth(sizeOfField);
			if (ImGui::InputTextString("",
				&dPtr->m_if))
			{
			}
			ImGui::PopItemWidth();
		}
	}
	ImGui::Spring(1);
	ImGui::EndVertical();
	auto contentRect = ImGui_GetItemRect();

	ImGui::Spring(1, padding);
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("outputs");
	ImGui::Spring(0, padding * 2);

	ImRect outputsRect;
	int outputAlpha = 175;
	if (myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id)->m_kind != Volt::BehaviorTree::eNodeKind::LEAF)
	{
		auto& pin = n.pins[1];
		ImGui::Dummy(ImVec2(1, padding));
		ImGui::Spring(1, 0);
		outputsRect = ImGui_GetItemRect();

		ed::PushStyleVar(ed::StyleVar_PinArrowSize, 0.0f);
		ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 0.0f);

		ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersTop);

		ed::BeginPin(ed::PinId(pin), ed::PinKind::Output);
		ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
		ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
		ed::EndPin();
		ed::PopStyleVar(3);
	}
	else ImGui::Dummy(ImVec2(0, padding));

	ImGui::Spring(0, padding * 2);
	ImGui::EndHorizontal();

	ImGui::EndVertical();

	ed::EndNode();
	ed::PopStyleVar(7);
	ed::PopStyleColor(4);

	auto drawList = ed::GetNodeBackgroundDrawList(ed::NodeId(n.id));

	const auto    topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
	const auto bottomRoundCornersFlags = ImDrawFlags_RoundCornersBottom;

	if (myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id)->m_kind != Volt::BehaviorTree::eNodeKind::ROOT)
	{
		drawList->AddRectFilled(
			inputsRect.GetTL() + ImVec2(0, 1),
			inputsRect.GetBR(),
			IM_COL32(
				(int)(0 * pinBackground.x),
				(int)(255 * pinBackground.y),
				(int)(0 * pinBackground.z),
				inputAlpha
			),
			4.0f,
			bottomRoundCornersFlags
		);

		drawList->AddRect(
			inputsRect.GetTL() + ImVec2(0, 1),
			inputsRect.GetBR(),
			IM_COL32(
				(int)(0 * pinBackground.x),
				(int)(255 * pinBackground.y),
				(int)(0 * pinBackground.z),
				inputAlpha
			),
			4.0f,
			bottomRoundCornersFlags
		);
	}

	if (myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id)->m_kind != Volt::BehaviorTree::eNodeKind::LEAF)
	{
		drawList->AddRectFilled(
			outputsRect.GetTL(),
			outputsRect.GetBR() - ImVec2(0, 1),
			IM_COL32(
				(int)(255 * pinBackground.x),
				(int)(0 * pinBackground.y),
				(int)(0 * pinBackground.z),
				outputAlpha
			),
			4.0f,
			topRoundCornersFlags
		);

		drawList->AddRect(
			outputsRect.GetTL(),
			outputsRect.GetBR() - ImVec2(0, 1),
			IM_COL32(
				(int)(255 * pinBackground.x),
				(int)(0 * pinBackground.y),
				(int)(0 * pinBackground.z),
				outputAlpha
			),
			4.0f,
			topRoundCornersFlags
		);
	}


	switch (myBehaviourTree->GetNodeManager().GetNodeFromUUID(n.id)->m_kind)
	{
		case Volt::BehaviorTree::eNodeKind::LEAF:
			drawList->AddRectFilled(
				contentRect.GetTL(),
				contentRect.GetBR(),
				IM_COL32(
					(int)(0 * pinBackground.x),
					(int)(255 * pinBackground.y),
					(int)(0 * pinBackground.z),
					inputAlpha),
				2.0f
			);
			break;

		case Volt::BehaviorTree::eNodeKind::ROOT:
			drawList->AddRectFilled(
				contentRect.GetTL(),
				contentRect.GetBR(),
				IM_COL32(
					(int)(255 * pinBackground.x),
					(int)(0 * pinBackground.y),
					(int)(0 * pinBackground.z),
					outputAlpha),
				2.0f
			);
			break;

		case Volt::BehaviorTree::eNodeKind::DECORATOR:
			drawList->AddRectFilled(
				contentRect.GetTL(),
				contentRect.GetBR(),
				IM_COL32(
					(int)(163 * pinBackground.x),
					(int)(73 * pinBackground.y),
					(int)(164 * pinBackground.z),
					outputAlpha),
				2.0f
			);
			break;

		case Volt::BehaviorTree::eNodeKind::SELECTOR:
			drawList->AddRectFilled(
				contentRect.GetTL(),
				contentRect.GetBR(),
				IM_COL32(
					(int)(255 * pinBackground.x),
					(int)(255 * pinBackground.y),
					(int)(0 * pinBackground.z),
					outputAlpha),
				2.0f
			);
			break;

		case Volt::BehaviorTree::eNodeKind::SEQUENCE:
			drawList->AddRectFilled(
				contentRect.GetTL(),
				contentRect.GetBR(),
				IM_COL32(
					(int)(0 * pinBackground.x),
					(int)(255 * pinBackground.y),
					(int)(255 * pinBackground.z),
					outputAlpha),
				2.0f
			);
			break;

		default:
			drawList->AddRectFilled(
				contentRect.GetTL(),
				contentRect.GetBR(),
				IM_COL32(29, 29, 29, 200),
				2.0f
			);
			break;
	}
}

OwnedPin BehaviorEditor::FindPin(ed::PinId id)
{
	OwnedPin ret;

	if (id.Get() == 0)
		return ret;
	for (auto& node : GetBackend().nodes)
	{
		if (node.pins[0] == id.Get())
		{
			ret.type = ePinType::INPUT;
			ret.nodeUUID = node.id;
			ret.pinUUID = node.pins[0];
		}
		if (node.pins[1] == id.Get())
		{
			ret.type = ePinType::OUTPUT;
			ret.nodeUUID = node.id;
			ret.pinUUID = node.pins[1];
		}
	}
	return ret;
}


void BehaviorEditor::DrawPanels()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	const std::string id = "Nodes##" + myContext;
	if (ImGui::Begin(id.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		ImGui::Text("Create new node: ");
		if (ImGui::Button("DECORATOR"))
		{
			ConstructNewNode<Volt::BehaviorTree::Decorator>();
		}
		if (ImGui::Button("SEQUENCE"))
		{
			ConstructNewNode<Volt::BehaviorTree::Sequence>();
		}
		if (ImGui::Button("SELECTOR"))
		{
			ConstructNewNode<Volt::BehaviorTree::Selector>();
		}
		if (ImGui::Button("LEAF"))
		{
			ConstructNewNode<Volt::BehaviorTree::Leaf>();
		}
		float i = ImGui::GetCurrentWindow()->Size.y;
		ImGui::Dummy({ 0, i - 250.5f });// 220.0f});//  172.5f });
		if (ImGui::Button("RUN"))
		{
			VT_CORE_ERROR("Run does not work, test in scene");
		}
		if (ImGui::Button("SAVE"))
		{
			const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(myBehaviourTree->handle);

			if (FileSystem::IsWriteable(Volt::ProjectManager::GetDirectory() / metadata.filePath))
			{
				UI::Notify(NotificationType::Success, "BehaviorTree saved", "");
				Volt::AssetManager::Get().SaveAsset(myBehaviourTree);
			}
			else
			{
				UI::Notify(NotificationType::Error, "BehaviorTree save failed", "");
			}
		}
		if (EditorUtils::Property("LOAD", m_currentHandle, Volt::AssetType::BehaviorGraph))
		{
			auto importedTree = Volt::AssetManager::GetAsset<Volt::BehaviorTree::Tree>(m_currentHandle);
			if (!importedTree || !importedTree->IsValid())
				return;
			GetBackend().links.clear();
			GetBackend().nodes.clear();
			for (const auto& _nodePair : importedTree->GetNodeManager().GetNodes())
			{
				AddNodeToBackend(_nodePair.first);
			}
			for (const auto& _linkPair : importedTree->GetNodeManager().GetLinkIDs())
			{
				NodeGraph::Link ngLink;
				ngLink.id = _linkPair.first;
				for (const auto& node : GetBackend().nodes)
				{
					if (_linkPair.second.m_parentID == node.id)
					{
						ngLink.output = node.pins[1];
					}
					if (_linkPair.second.m_childID == node.id)
					{
						ngLink.input = node.pins[0];
					}
				}
				GetBackend().links.emplace_back(ngLink);
			}
			InitializeEditor(nullptr);
			myBehaviourTree = importedTree;
		}
	}
	ImGui::End();
}

void BehaviorEditor::DrawContextPopups()
{
	if (UI::BeginPopup("New Node"))
	{
		ImGui::Text("Create new node: ");
		if (ImGui::Button("DECORATOR"))
		{
			ConstructNewNode<Volt::BehaviorTree::Decorator>();
			auto newNodePostion = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup());
			ed::SetNodePosition(ed::NodeId(GetBackend().nodes.back().id), newNodePostion);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("SEQUENCE"))
		{
			ConstructNewNode<Volt::BehaviorTree::Sequence>();
			auto newNodePostion = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup());
			ed::SetNodePosition(ed::NodeId(GetBackend().nodes.back().id), newNodePostion);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("SELECTOR"))
		{
			ConstructNewNode<Volt::BehaviorTree::Selector>();
			auto newNodePostion = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup());
			ed::SetNodePosition(ed::NodeId(GetBackend().nodes.back().id), newNodePostion);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("LEAF"))
		{
			ConstructNewNode<Volt::BehaviorTree::Leaf>();
			auto newNodePostion = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup());
			ed::SetNodePosition(ed::NodeId(GetBackend().nodes.back().id), newNodePostion);
			ImGui::CloseCurrentPopup();
		}
		UI::EndPopup();
	}

	if (UI::BeginPopup("Decorator thing"))
	{
		for (int i = 0; i < 5; i++)
		{
			if (ImGui::Selectable(Volt::BehaviorTree::decBfr[i].c_str(), (int)m_decPtr->m_type == i))
			{
				m_decPtr->m_type = static_cast<Volt::BehaviorTree::eDecoratorType>(i);
			}
		}
		UI::EndPopup();
	}
}


bool BehaviorEditor::SaveSettings(const std::string& data)
{
	return false;
}

size_t BehaviorEditor::LoadSettings(std::string& data)
{
	return size_t();
}

bool BehaviorEditor::SaveNodeSettings(const Volt::UUID nodeId, const std::string& data)
{
	auto node = myBehaviourTree->GetNodeManager().GetNodeFromUUID(nodeId);
	if (!node)
		return false;
	node->SetPos(data);
	SortOutput();
	return true;
}

size_t BehaviorEditor::LoadNodeSettings(const Volt::UUID nodeId, std::string& data)
{
	auto node = myBehaviourTree->GetNodeManager().GetNodeFromUUID(nodeId);
	if (!node)
		return 0;
	data = node->GetPos();
	return data.size();
}
