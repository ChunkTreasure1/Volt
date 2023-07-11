#include "sbpch.h"
#include "AnimationGraphPanel.h"

#include "Sandbox/Utility/NodeEditorHelpers.h"

#include <Volt/Animation/AnimationStateMachine.h>
#include <Volt/Animation/AnimationTransitionGraph.h>
#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Importers/TextureImporter.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Utility/UIUtility.h>

#include <GraphKey/Nodes/Animation/StateMachineNodes.h>
#include <GraphKey/Nodes/Animation/BlendNodes.h>

#include <builders.h>
#include <typeindex>
#include <ranges>

inline static ImRect ImGui_GetItemRect()
{
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

AnimationGraphPanel::AnimationGraphPanel(Ref<Volt::Scene>& currentScene)
	: IONodeGraphEditor("Animation Graph", "Animation Graph", currentScene)
{
}

AnimationGraphPanel::~AnimationGraphPanel()
{
}

void AnimationGraphPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	myGraphDepth.clear();
	myCurrentAsset = std::reinterpret_pointer_cast<Volt::AnimationGraphAsset>(asset);

	const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(asset->handle);

	auto& graphDepth = myGraphDepth.emplace_back();
	graphDepth.editorType = EditorType::Graph;
	graphDepth.name = (!metadata.filePath.empty()) ? metadata.filePath.stem().string() : "New Graph";
	graphDepth.graph = myCurrentAsset;

	myOpenGraph = graphDepth.graph;
	ReconstructGraph();
}

bool AnimationGraphPanel::SaveSettings(const std::string& data)
{
	if (!myCurrentAsset)
	{
		return false;
	}

	if (GetLastEntry().editorType == EditorType::Graph)
	{
		if (auto animGraph = std::dynamic_pointer_cast<Volt::AnimationGraphAsset>(myOpenGraph))
		{
			animGraph->SetState(data);
		}
		else if (auto transitionGraph = std::dynamic_pointer_cast<Volt::AnimationTransitionGraph>(myOpenGraph))
		{
			transitionGraph->SetState(data);
		}
	}
	else
	{
		GetLastEntry().stateMachine->SetEditorState(data);
	}

	return true;
}

size_t AnimationGraphPanel::LoadSettings(std::string& data)
{
	if (!myCurrentAsset)
	{
		return 0;
	}

	if (GetLastEntry().editorType == EditorType::Graph)
	{
		if (auto animGraph = std::dynamic_pointer_cast<Volt::AnimationGraphAsset>(myOpenGraph))
		{
			data = animGraph->GetState();
			return data.size();
		}
		else if (auto transitionGraph = std::dynamic_pointer_cast<Volt::AnimationTransitionGraph>(myOpenGraph))
		{
			data = transitionGraph->GetState();
			return data.size();
		}
	}
	else
	{
		data = GetLastEntry().stateMachine->GetEditorState();
		return data.size();
	}

	return 0;
}

bool AnimationGraphPanel::SaveNodeSettings(const Volt::UUID nodeId, const std::string& data)
{
	if (GetLastEntry().editorType == EditorType::Graph)
	{
		auto node = myOpenGraph->GetNodeByID(nodeId);
		if (!node)
		{
			return false;
		}

		node->editorState = data;
	}
	else
	{
		auto state = GetLastEntry().stateMachine->GetStateById(nodeId);
		if (!state)
		{
			return false;
		}
		state->editorState = data;
	}

	return true;
}

size_t AnimationGraphPanel::LoadNodeSettings(const Volt::UUID nodeId, std::string& data)
{
	if (GetLastEntry().editorType == EditorType::Graph)
	{
		auto node = myOpenGraph->GetNodeByID(nodeId);
		if (!node)
		{
			return 0;
		}

		data = node->editorState;
		return data.size();
	}
	else
	{
		auto state = GetLastEntry().stateMachine->GetStateById(nodeId);
		if (!state)
		{
			return 0;
		}

		data = state->editorState;
		return data.size();
	}
}

void AnimationGraphPanel::DrawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Graph"))
			{
				myNewAnimGraphData = {};
				UI::OpenModal("New Graph##AnimGraphEditor");
			}

			if (ImGui::MenuItem("Open"))
			{
				const std::filesystem::path path = FileSystem::OpenFileDialogue({ { "Animation Graph (*.vtanimgraph)", "vtanimgraph" } });
				if (!path.empty() && FileSystem::Exists(path))
				{
					myCurrentAsset = Volt::AssetManager::GetAsset<Volt::AnimationGraphAsset>(path);
					myOpenGraph = myCurrentAsset;

					const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(myCurrentAsset->handle);

					myGraphDepth.clear();
					auto& graphDepth = myGraphDepth.emplace_back();
					graphDepth.editorType = EditorType::Graph;
					graphDepth.name = (!metadata.filePath.empty()) ? metadata.filePath.stem().string() : "New Graph";
					graphDepth.graph = myCurrentAsset;

					ReconstructGraph();
				}
			}

			if (ImGui::MenuItem("Save"))
			{
				if (myOpenGraph)
				{
					Volt::AssetManager::Get().SaveAsset(myCurrentAsset);
					const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(myCurrentAsset->handle);

					UI::Notify(NotificationType::Success, "Saved character!", std::format("Character {0} successfully saved!", metadata.filePath.string()));
				}
			}

			if (ImGui::MenuItem("Save As"))
			{
				if (myOpenGraph)
				{
					const std::filesystem::path targetPath = FileSystem::SaveFileDialogue({ { "Animated Character (*.vtchr)", "vtchr"} });
					if (!targetPath.empty())
					{
						Volt::AssetManager::SaveAssetAs(myCurrentAsset, targetPath);
					}
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void AnimationGraphPanel::DrawPanels()
{
	IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>::DrawPanels();
	DrawGraphDepthBar();
	DrawPropertiesPanel();

	if (myShouldReconstruct)
	{
		myShouldReconstruct = false;
		ReconstructGraph();
	}
}

void AnimationGraphPanel::DrawNodes()
{
	if (myGraphDepth.empty())
	{
		return;
	}

	if (GetLastEntry().editorType == EditorType::Graph)
	{
		if (!myOpenGraph)
		{
			return;
		}

		auto id = ed::GetDoubleClickedNode();
		auto node = myOpenGraph->GetNodeByID(id.Get());
		if (node)
		{
			if (auto stateType = std::dynamic_pointer_cast<GraphKey::StateMachineNode>(node))
			{
				auto& newEntry = myGraphDepth.emplace_back();
				newEntry.name = stateType->GetStateMachine()->GetName();
				newEntry.editorType = EditorType::StateMachine;
				newEntry.stateMachine = stateType->GetStateMachine();

				return;
			}
		}

		IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>::DrawNodes();
	}
	else
	{
		if (GetLastEntry().editorType == EditorType::StateMachine)
		{
			// Transition
			{
				auto id = ed::GetDoubleClickedLink();
				auto transition = GetLastEntry().stateMachine->GetTransitionById(id.Get());
				if (transition && transition->transitionGraph)
				{
					auto& newEntry = myGraphDepth.emplace_back();
					newEntry.name = "Transition Graph";
					newEntry.editorType = EditorType::Graph;
					newEntry.graph = transition->transitionGraph;

					transition->transitionGraph->SetParentBlackboard(&myCurrentAsset->GetBlackboard());

					myOpenGraph = transition->transitionGraph;
					myShouldReconstruct = true;

					return;
				}
			}

			// Node
			{
				auto id = ed::GetDoubleClickedNode();
				auto state = GetLastEntry().stateMachine->GetStateById(id.Get());

				if (state && state->stateGraph)
				{
					auto& newEntry = myGraphDepth.emplace_back();
					newEntry.name = state->name + " Graph";
					newEntry.editorType = EditorType::Graph;
					newEntry.graph = state->stateGraph;

					state->stateGraph->SetParentBlackboard(&myCurrentAsset->GetBlackboard());

					myOpenGraph = state->stateGraph;
					myShouldReconstruct = true;

					return;
				}
			}
		}

		DrawStateMachineNodes();
	}
}

void AnimationGraphPanel::DrawLinks()
{
	if (myGraphDepth.empty())
	{
		return;
	}

	if (GetLastEntry().editorType == EditorType::Graph)
	{
		IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>::DrawLinks();
		return;
	}

	auto stateMachine = GetLastEntry().stateMachine;

	std::unordered_map<LinkPair, std::array<Ref<Volt::AnimationTransition>, 2>> stateTransitionMap;

	for (const auto& t : stateMachine->GetTransitions())
	{
		// Sort transitions per state (no more than 2 per state)
		LinkPair pairFromTo{ t->fromState, t->toState };
		LinkPair pairToFrom{ t->toState, t->fromState };

		if (!stateTransitionMap.contains(pairToFrom) && !stateTransitionMap.contains(pairFromTo))
		{
			stateTransitionMap.emplace(pairFromTo, std::array<Ref<Volt::AnimationTransition>, 2>{ nullptr, nullptr });
		}
		else if (!stateTransitionMap.contains(pairFromTo))
		{
			std::swap(pairFromTo, pairToFrom);
		}

		if (!stateTransitionMap[pairFromTo][0])
		{
			stateTransitionMap[pairFromTo][0] = t;
		}
		else
		{
			stateTransitionMap[pairFromTo][1] = t;
		}
	}

	for (const auto& [pair, transitions] : stateTransitionMap)
	{
		// pinId is top, pinId2 is bottom
		ImVec2 offset = transitions.at(1) ? ImVec2{ 10.f, 0.f } : ImVec2{ 0.f, 0.f };

		{
			auto t = transitions.at(0);

			auto* fromState = stateMachine->GetStateById(t->fromState);
			auto* toState = stateMachine->GetStateById(t->toState);

			const auto fromPos = ed::GetNodePosition(ed::NodeId(fromState->id));
			const auto toPos = ed::GetNodePosition(ed::NodeId(toState->id));

			if (fromPos.y < toPos.y)
			{
				ed::Link(ed::LinkId(t->id), ed::PinId(fromState->pinId2), ed::PinId(toState->pinId), offset, ax::NodeEditor::ArrowLocation::End);
			}
			else
			{
				ed::Link(ed::LinkId(t->id), ed::PinId(fromState->pinId), ed::PinId(toState->pinId2), offset, ax::NodeEditor::ArrowLocation::End);
			}
		}

		{
			if (!transitions.at(1))
			{
				continue;
			}
			auto t = transitions.at(1);

			auto* fromState = stateMachine->GetStateById(t->fromState);
			auto* toState = stateMachine->GetStateById(t->toState);

			const auto fromPos = ed::GetNodePosition(ed::NodeId(fromState->id));
			const auto toPos = ed::GetNodePosition(ed::NodeId(toState->id));

			if (fromPos.y < toPos.y)
			{
				ed::Link(ed::LinkId(t->id), ed::PinId(fromState->pinId2), ed::PinId(toState->pinId), offset * -1.f, ax::NodeEditor::ArrowLocation::End);
			}
			else
			{
				ed::Link(ed::LinkId(t->id), ed::PinId(fromState->pinId), ed::PinId(toState->pinId2), offset * -1.f, ax::NodeEditor::ArrowLocation::End);
			}
		}
	}
}

void AnimationGraphPanel::DrawNodesPanel()
{
	if (myGraphDepth.empty())
	{
		return;
	}

	if (GetLastEntry().editorType == EditorType::Graph)
	{
		IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>::DrawNodesPanel();
		return;
	}

	ImGui::SetNextWindowClass(GetWindowClass());

	const std::string id = "Nodes##" + myContext;
	if (ImGui::Begin(id.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
	{
		if (ImGui::Button("Add state"))
		{
			GetLastEntry().stateMachine->AddState("New State", false);
		}
	}
	ImGui::End();
}

void AnimationGraphPanel::OnBeginCreate()
{
	if (GetLastEntry().editorType == EditorType::Graph)
	{
		IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>::OnBeginCreate();
		return;
	}

	OnBeginCreateStateMachine();
}

void AnimationGraphPanel::OnDeleteLink(const Volt::UUID id)
{
	if (GetLastEntry().editorType == EditorType::Graph)
	{
		IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>::OnDeleteLink(id);
		return;
	}
	else
	{
		auto stateMachine = GetLastEntry().stateMachine;
		stateMachine->RemoveTransition(id);
	}
}

void AnimationGraphPanel::OnDeleteNode(const Volt::UUID id)
{
	if (GetLastEntry().editorType == EditorType::Graph)
	{
		IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>::OnDeleteNode(id);
		return;
	}
	else
	{
		auto stateMachine = GetLastEntry().stateMachine;
		auto state = stateMachine->GetStateById(id);
		if (state && !state->isEntry && !state->isAny)
		{
			stateMachine->RemoveState(id);
		}
	}
}

void AnimationGraphPanel::DrawPropertiesPanel()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	if (ImGui::Begin("Properties##animationGraphPanel"))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		UI::Header("Graph Properties");
		ImGui::Separator();

		if (myCurrentAsset)
		{
			if (UI::BeginProperties("graphProperties"))
			{
				auto handle = myCurrentAsset->GetCharacterHandle();
				if (EditorUtils::Property("Character", handle, Volt::AssetType::AnimatedCharacter))
				{
					myCurrentAsset->SetCharacterHandle(handle);
				}


				UI::EndProperties();
			}
		}

		UI::Header("Properties");
		ImGui::Separator();

		if (!myGraphDepth.empty())
		{
			if (GetLastEntry().editorType == EditorType::Graph)
			{
				DrawGraphProperties();
			}
			else
			{
				DrawStateMachineProperties();
			}
		}
	}
	ImGui::End();
}

void AnimationGraphPanel::DrawGraphDepthBar()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	if (ImGui::Begin("##graphDepthBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTabBar))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		if (!myOpenGraph)
		{
			ImGui::End();
			return;
		}

		int32_t removeIndex = -1;

		for (uint32_t i = 0; const auto & entry : myGraphDepth)
		{
			const std::string id = std::format("{0}##{1}", entry.name, i);

			if (ImGui::Button(id.c_str()))
			{
				if (entry.editorType == EditorType::Graph)
				{
					myOpenGraph = entry.graph;
				}

				myShouldReconstruct = true;

				removeIndex = static_cast<int32_t>(i + 1);
				break;
			}

			if (i < static_cast<uint32_t>(myGraphDepth.size() - 1))
			{
				ImGui::SameLine();
				ImGui::TextUnformatted("->");
				ImGui::SameLine();
			}

			i++;
		}

		if (removeIndex > 0)
		{
			for (int32_t i = static_cast<int32_t>(myGraphDepth.size()); i > removeIndex; i--)
			{
				myGraphDepth.pop_back();
			}
		}
	}
	ImGui::End();
}

void AnimationGraphPanel::DrawStateMachineNodes()
{
	auto stateMachine = GetLastEntry().stateMachine;

	if (!stateMachine)
	{
		return;
	}

	constexpr float rounding = 2.0f;
	constexpr float padding = 12.0f;
	const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_Bg];

	for (const auto& state : stateMachine->GetStates())
	{
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
		ed::BeginNode(ed::NodeId(state->id));

		ImRect inputsRect;
		ImRect outputsRect;
		ImRect contentRect;
		int inputAlpha = 175;

		// Inputs
		{
			ImGui::BeginVertical(ed::NodeId(state->id).AsPointer());
			ImGui::BeginHorizontal("inputs");

			ImGui::Dummy(ImVec2(1, padding));
			ImGui::Spring(1, 0);
			inputsRect = ImGui_GetItemRect();

			ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.f);
			ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
			ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);

			ed::BeginPin(ed::PinId(state->pinId), ed::PinKind::Input);
			ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
			ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
			ed::EndPin();
			ed::PopStyleVar(3);

			ImGui::EndHorizontal();
		}

		// Content
		{
			ImGui::BeginHorizontal("content_frame");
			ImGui::Spring(1, padding);

			ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
			ImGui::Dummy(ImVec2(160, 0));
			ImGui::Spring(1);
			ImGui::TextUnformatted(state->name.c_str());
			ImGui::Spring(1);
			ImGui::EndVertical();
			contentRect = ImGui_GetItemRect();

			ImGui::Spring(1, padding);
			ImGui::EndHorizontal();
		}

		int outputAlpha = 175;

		// Outputs
		{
			ImGui::BeginHorizontal("outputs");
			ImGui::Dummy(ImVec2(1, padding));
			ImGui::Spring(1, 0);
			outputsRect = ImGui_GetItemRect();

			ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.f);
			ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
			ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);
			ed::BeginPin(ed::PinId(state->pinId2), ed::PinKind::Output);
			ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
			ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
			ed::EndPin();

			ed::PopStyleVar(3);

			ImGui::EndHorizontal();
		}

		ImGui::EndVertical();
		ed::EndNode();
		ed::PopStyleVar(7);
		ed::PopStyleColor(4);

		auto drawList = ed::GetNodeBackgroundDrawList(ed::NodeId(state->id));

		const auto    topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
		const auto bottomRoundCornersFlags = ImDrawFlags_RoundCornersBottom;

		drawList->AddRectFilled(
			inputsRect.GetTL() + ImVec2(0, 1),
			inputsRect.GetBR(),
			IM_COL32(
				(int)(255 * pinBackground.x),
				(int)(255 * pinBackground.y),
				(int)(255 * pinBackground.z),
				inputAlpha
			),
			4.0f,
			bottomRoundCornersFlags
		);

		drawList->AddRect(
			inputsRect.GetTL() + ImVec2(0, 1),
			inputsRect.GetBR(),
			IM_COL32(
				(int)(255 * pinBackground.x),
				(int)(255 * pinBackground.y),
				(int)(255 * pinBackground.z),
				inputAlpha
			),
			4.0f,
			bottomRoundCornersFlags
		);

		drawList->AddRectFilled(
			outputsRect.GetTL(),
			outputsRect.GetBR() - ImVec2(0, 1),
			IM_COL32(
				(int)(255 * pinBackground.x),
				(int)(255 * pinBackground.y),
				(int)(255 * pinBackground.z),
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
				(int)(255 * pinBackground.y),
				(int)(255 * pinBackground.z),
				outputAlpha
			),
			4.0f,
			topRoundCornersFlags
		);


		auto mainColor = IM_COL32(29, 29, 29, 200);

		if (state->isEntry)
		{
			mainColor = IM_COL32(255, 174, 0, 255);
		}
		else if (state->isAny)
		{
			mainColor = IM_COL32(92, 171, 255, 255);
		}

		drawList->AddRectFilled(
			contentRect.GetTL(),
			contentRect.GetBR(),
			mainColor,
			2.0f
		);
	}
}

void AnimationGraphPanel::OnBeginCreateStateMachine()
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

	auto stateMachine = GetLastEntry().stateMachine;

	ed::PinId endPinId = 0, startPinId = 0;
	if (ed::QueryNewLink(&startPinId, &endPinId))
	{
		auto* startState = stateMachine->GetStateFromPin(startPinId.Get());
		auto* endState = stateMachine->GetStateFromPin(endPinId.Get());

		if (startPinId.Get() == endPinId.Get())
		{
			ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
		}

		if (startState && endState)
		{
			uint32_t transitionsToEndCount = 0;
			for (const auto& t : startState->transitions)
			{
				auto it = std::find_if(endState->transitions.begin(), endState->transitions.end(), [&](const auto& lhs)
				{
					return lhs == t;
				});

				if (it != endState->transitions.end())
				{
					transitionsToEndCount++;
				}
			}

			auto sameStateIt = std::find_if(startState->transitions.begin(), startState->transitions.end(), [&](const auto& lhs)
			{
				auto transition = stateMachine->GetTransitionById(lhs);
				if (transition)
				{
					return transition->fromState == startState->id && transition->toState == endState->id;
				}

				return false;
			});

			if (transitionsToEndCount > 1 || sameStateIt != startState->transitions.end())
			{
				ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
				showLabel("x States are already connected", ImColor{ 255, 0, 0 });
			}
			else if (endState->isEntry)
			{
				ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
				showLabel("x You cannot go to Entry state!", ImColor{ 255, 0, 0 });
			}
			else if (endState->isAny)
			{
				ed::RejectNewItem(ImColor{ 255, 0, 0 }, 2.f);
				showLabel("x You cannot go to Any state!", ImColor{ 255, 0, 0 });
			}

			if (ed::AcceptNewItem(ImColor{ 1.f, 1.f, 1.f }, 2.f) && sameStateIt == startState->transitions.end() && !endState->isEntry && !endState->isAny)
			{
				stateMachine->AddTransition(startState->id, endState->id);
			}
		}
	}
}

void AnimationGraphPanel::DrawGraphProperties()
{
	const auto& selectedNodes = GetSelectedNodes();
	if (selectedNodes.size() != 1)
	{
		return;
	}

	auto node = GetLastEntry().graph->GetNodeByID(selectedNodes.at(0));
	if (!node)
	{
		return;
	}

	UI::PushId();
	if (UI::BeginProperties("graphProperties"))
	{
		if (node->GetRegistryName() == "StateMachineNode")
		{
			auto stateMachineNode = std::reinterpret_pointer_cast<GraphKey::StateMachineNode>(node);
			if (stateMachineNode->GetStateMachine())
			{
				std::string name = stateMachineNode->GetStateMachine()->GetName();
				if (UI::Property("Name", name))
				{
					stateMachineNode->GetStateMachine()->SetName(name);
				}
			}
		}

		auto handle = myCurrentAsset->GetCharacterHandle();
		if (EditorUtils::Property("Character", handle, Volt::AssetType::AnimatedCharacter))
		{
			myCurrentAsset->SetCharacterHandle(handle);
		}

		UI::EndProperties();
	}
	UI::PopId();

	if (node->GetRegistryName() == "LayeredBlendPerBoneNode")
	{
		DrawLayeredBlendPerBoneProperties(node);
	}
}

void AnimationGraphPanel::DrawStateMachineProperties()
{
	const auto& selectedNodes = GetSelectedStates();
	const auto& selectedTransition = GetSelectedTransitions();

	if ((!selectedNodes.empty() && !selectedTransition.empty()) ||
		(selectedNodes.size() != 1 && selectedTransition.size() != 1))
	{
		return;
	}

	if (!selectedTransition.empty())
	{
		auto transition = GetLastEntry().stateMachine->GetTransitionById(selectedTransition.at(0));
		if (!transition)
		{
			return;
		}

		UI::PushId();
		if (UI::BeginProperties("transitionProperties"))
		{
			UI::Property("Has Exit Time", transition->hasExitTime);
			if (transition->hasExitTime)
			{
				UI::Property("Blend Start Time", transition->exitStartValue);
			}

			UI::Property("Should Blend", transition->shouldBlend);

			if (transition->shouldBlend)
			{
				UI::Property("Blend Time", transition->blendTime);
			}

			UI::EndProperties();
		}
		UI::PopId();
	}
	else
	{
		auto node = GetLastEntry().stateMachine->GetStateById(selectedNodes.at(0));
		if (!node)
		{
			return;
		}

		UI::PushId();
		if (UI::BeginProperties("stateProperties"))
		{
			if (node->name != "Entry")
			{
				UI::Property("Name", node->name);
			}

			UI::EndProperties();
		}
		UI::PopId();
	}

}

void AnimationGraphPanel::DrawLayeredBlendPerBoneProperties(Ref<GraphKey::Node> node)
{
	Ref<GraphKey::LayeredBlendPerBoneNode> blendNode = std::reinterpret_pointer_cast<GraphKey::LayeredBlendPerBoneNode>(node);

	auto& includeFilters = blendNode->GetBlendIncludeFilters();
	auto& exlucdeFilters = blendNode->GetBlendExcludeFilters();

	UI::Header("Included Joint Chains");
	ImGui::Separator();

	{
		UI::ScopedButtonColor buttCol{ EditorTheme::Buttons::AddButton };
		if (ImGui::Button("Add +##paramInc"))
		{
			includeFilters.emplace_back();
		}
	}

	if (UI::BeginProperties("blendFilters"))
	{
		for (auto& blendFilter : includeFilters)
		{
			UI::Property("Filter", blendFilter.boneName);
		}
		UI::EndProperties();
	}

	UI::Header("Excluded Joint Chains");
	ImGui::Separator();

	{
		UI::ScopedButtonColor buttCol{ EditorTheme::Buttons::AddButton };
		if (ImGui::Button("Add +##paramEx"))
		{
			exlucdeFilters.emplace_back();
		}
	}

	if (UI::BeginProperties("blendFilters"))
	{
		for (auto& blendFilter : exlucdeFilters)
		{
			UI::Property("Filter", blendFilter.boneName);
		}
		UI::EndProperties();
	}

}

const std::vector<Volt::UUID> AnimationGraphPanel::GetSelectedStates() const
{
	std::vector<ed::NodeId> selectedNodeIds;
	selectedNodeIds.resize(ed::GetSelectedObjectCount());

	int32_t nodeCount = ed::GetSelectedNodes(selectedNodeIds.data(), (int32_t)selectedNodeIds.size());
	selectedNodeIds.resize(nodeCount);

	auto stateMachine = GetLastEntry().stateMachine;

	std::vector<Volt::UUID> result;
	for (const auto& n : stateMachine->GetStates())
	{
		auto it = std::find_if(selectedNodeIds.begin(), selectedNodeIds.end(), [&](const ed::NodeId& id)
		{
			return id.Get() == n->id;
		});

		if (it != selectedNodeIds.end())
		{
			result.emplace_back(n->id);
		}
	}

	return result;
}

const std::vector<Volt::UUID> AnimationGraphPanel::GetSelectedTransitions() const
{
	std::vector<ed::LinkId> selectedLinkIds;
	selectedLinkIds.resize(ed::GetSelectedObjectCount());

	int32_t linkCount = ed::GetSelectedLinks(selectedLinkIds.data(), (int32_t)selectedLinkIds.size());
	selectedLinkIds.resize(linkCount);

	auto stateMachine = GetLastEntry().stateMachine;

	std::vector<Volt::UUID> result;
	for (const auto& l : stateMachine->GetTransitions())
	{
		auto it = std::find_if(selectedLinkIds.begin(), selectedLinkIds.end(), [&](const ed::LinkId& id)
		{
			return id.Get() == l->id;
		});

		if (it != selectedLinkIds.end())
		{
			result.emplace_back(l->id);
		}
	}

	return result;
}
