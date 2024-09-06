#include "sbpch.h"
#include "Window/MosaicEditor/MosaicEditorPanel.h"

#include "Sandbox/NodeGraph/NodeGraphEditorPinUtility.h"
#include "Sandbox/NodeGraph/IONodeGraphEditorHelpers.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Asset/Rendering/Material.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Project/ProjectManager.h>

#include <AssetSystem/AssetManager.h>

#include <Mosaic/MosaicGraph.h>
#include <Mosaic/MosaicNode.h>
#include <Mosaic/NodeRegistry.h>

#include <builders.h>

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

namespace Utility
{
	inline bool IsSameType(const Mosaic::TypeInfo& lhs, const Mosaic::TypeInfo& rhs)
	{
		const bool sameBaseType = lhs.baseType == rhs.baseType;
		const bool sameVectorSize = lhs.vectorSize == rhs.vectorSize;
		const bool sameColumnCount = lhs.columnCount == rhs.columnCount;

		return sameBaseType && sameVectorSize && sameColumnCount;
	}

	inline bool IsParameterLinked(Mosaic::MosaicGraph& graph, const UUID64 paramId)
	{
		for (const auto& edge : graph.GetUnderlyingGraph().GetEdges())
		{
			const auto& endNode = graph.GetUnderlyingGraph().GetNodeFromID(edge.endNode);
			const auto& endParam = endNode.nodeData->GetInputParameter(edge.metaDataType->GetParameterInputIndex());

			if (endParam.id == paramId)
			{
				return true;
			}

			const auto& startNode = graph.GetUnderlyingGraph().GetNodeFromID(edge.startNode);
			const auto& startParam = startNode.nodeData->GetOutputParameter(edge.metaDataType->GetParameterOutputIndex());

			if (startParam.id == paramId)
			{
				return true;
			}
		}

		return false;
	}

	inline void ClearLinksFromParameter(Mosaic::MosaicGraph& graph, const UUID64 paramId)
	{
		Vector<UUID64> edgesToRemove;

		for (const auto& edge : graph.GetUnderlyingGraph().GetEdges())
		{
			const auto& endNode = graph.GetUnderlyingGraph().GetNodeFromID(edge.endNode);
			const auto& endParam = endNode.nodeData->GetInputParameter(edge.metaDataType->GetParameterInputIndex());

			if (endParam.id == paramId)
			{
				edgesToRemove.emplace_back(edge.id);
			}

			const auto& startNode = graph.GetUnderlyingGraph().GetNodeFromID(edge.startNode);
			const auto& startParam = startNode.nodeData->GetOutputParameter(edge.metaDataType->GetParameterOutputIndex());

			if (startParam.id == paramId)
			{
				edgesToRemove.emplace_back(edge.id);
			}
		}

		for (const auto& edge : edgesToRemove)
		{
			graph.GetUnderlyingGraph().RemoveEdge(edge);
		}
	}

	inline void DrawPinIcon(const Mosaic::Parameter& param, bool connected, ImColor color, int32_t alpha)
	{
		IconType iconType = IconType::Circle;

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

	inline void DrawAttribute(Mosaic::Parameter& parameter)
	{
		if (parameter.typeInfo.baseType == Mosaic::ValueBaseType::Float)
		{
			if (parameter.typeInfo.vectorSize == 1)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<float>());
			}
			else if (parameter.typeInfo.vectorSize == 2)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<glm::vec2>());
			}
			else if (parameter.typeInfo.vectorSize == 3)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<glm::vec3>());
			}
			else if (parameter.typeInfo.vectorSize == 4)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<glm::vec4>());
			}
		}
		else if (parameter.typeInfo.baseType == Mosaic::ValueBaseType::Int)
		{
			if (parameter.typeInfo.vectorSize == 1)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<int32_t>());
			}
			else if (parameter.typeInfo.vectorSize == 2)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<glm::ivec2>());
			}
			else if (parameter.typeInfo.vectorSize == 3)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<glm::ivec3>());
			}
			else if (parameter.typeInfo.vectorSize == 4)
			{
				IONodeGraphEditorHelpers::Attribute(parameter.Get<glm::ivec4>());
			}
		}
	}

	inline glm::vec4 GetColorFromTypeInfo(const Mosaic::TypeInfo& typeInfo)
	{
		return { 1.f };
	}

	inline UUID64 GetNodeIdFromParameter(const Mosaic::MosaicGraph& graph, const UUID64 paramId)
	{
		for (const auto& node : graph.GetUnderlyingGraph().GetNodes())
		{
			for (const auto& param : node.nodeData->GetInputParameters())
			{
				if (param.id == paramId)
				{
					return node.id;
				}
			}

			for (const auto& param : node.nodeData->GetOutputParameters())
			{
				if (param.id == paramId)
				{
					return node.id;
				}
			}
		}

		return 0;
	}
}

MosaicEditorPanel::MosaicEditorPanel()
	: EditorWindow("Mosaic Editor", true)
{
	m_windowFlags = ImGuiWindowFlags_MenuBar;

	//auto& mosaicGraph = m_mosaicAsset->GetGraph();

	//auto constantNode = mosaicGraph.m_graph.AddNode(CreateRef<Mosaic::ConstantNode<glm::vec4, Mosaic::ValueBaseType::Float, 4, "{75DD7D2C-3B74-48C4-9F5C-062EBF53F44A}"_guid>>(mosaicGraph.get()));
	//auto addNode = mosaicGraph.m_graph.AddNode(CreateRef<Mosaic::AddNode>(mosaicGraph.get()));
	//auto sampleTextureNode = mosaicGraph.m_graph.AddNode(CreateRef<Mosaic::SampleTextureNode>(mosaicGraph.get()));

	//auto outputNode = mosaicGraph.m_graph.AddNode(CreateRef<Mosaic::PBROutputNode>(mosaicGraph.get()));

	//mosaicGraph.m_graph.LinkNodes(constantNode, addNode, CreateRef<Mosaic::MosaicEdge>(0, 0));
	//mosaicGraph.m_graph.LinkNodes(addNode, outputNode, CreateRef<Mosaic::MosaicEdge>(1, 0));
	//mosaicGraph.m_graph.LinkNodes(sampleTextureNode, outputNode, CreateRef<Mosaic::MosaicEdge>(0, 5));

	InitializeEditor();
}

MosaicEditorPanel::~MosaicEditorPanel()
{
	if (m_context.editorContext)
	{
		ed::DestroyEditor(m_context.editorContext);
	}
}

void MosaicEditorPanel::UpdateMainContent()
{
	DrawMenuBar();
}

void MosaicEditorPanel::UpdateContent()
{
	DrawEditor();

	ed::SetCurrentEditor(m_context.editorContext);
	DrawPanels();
	ed::SetCurrentEditor(nullptr);
}

bool MosaicEditorPanel::SaveSettings(const std::string& data)
{
	if (!m_material)
	{
		return false;
	}

	m_material->GetGraph().GetEditorState() = data;
	return true;
}

size_t MosaicEditorPanel::LoadSettings(std::string& data)
{
	if (!m_material)
	{
		return 0;
	}

	data = m_material->GetGraph().GetEditorState();
	return data.size();
}

bool MosaicEditorPanel::SaveNodeSettings(const UUID64 nodeId, const std::string& data)
{
	if (!m_material)
	{
		return false;
	}

	auto& node = m_material->GetGraph().GetUnderlyingGraph().GetNodeFromID(nodeId);
	if (!node.IsValid())
	{
		return false;
	}

	node.nodeData->GetEditorState() = data;
	return true;
}

size_t MosaicEditorPanel::LoadNodeSettings(const UUID64 nodeId, std::string& data)
{
	if (!m_material)
	{
		return 0;
	}

	const auto& node = m_material->GetGraph().GetUnderlyingGraph().GetNodeFromID(nodeId);
	if (!node.IsValid())
	{
		return 0;
	}

	data = node.nodeData->GetEditorState();

	return data.size();
}

void MosaicEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	m_material = std::reinterpret_pointer_cast<Volt::Material>(asset);
}

void MosaicEditorPanel::OnClose()
{
	m_material = nullptr;
}

const MosaicEditorPanel::IncompatiblePinReason MosaicEditorPanel::CanLinkPins(const UUID64 startParamId, const UUID64 endParamId)
{
	if (startParamId == endParamId)
	{
		return IncompatiblePinReason::SamePin;
	}

	auto* startParam = &GetParameterFromID(startParamId);
	auto* endParam = &GetParameterFromID(endParamId);

	if (startParam->id == 0 || endParam->id == 0)
	{
		return IncompatiblePinReason::None;
	}

	if (startParam->direction == Mosaic::ParameterDirection::Input)
	{
		std::swap(startParam, endParam);
	}

	if (!Utility::IsSameType(startParam->typeInfo, endParam->typeInfo) && startParam->typeInfo.baseType != Mosaic::ValueBaseType::Dynamic && endParam->typeInfo.baseType != Mosaic::ValueBaseType::Dynamic)
	{
		return IncompatiblePinReason::IncompatibleType;
	}

	return IncompatiblePinReason::None;
}

Mosaic::Parameter& MosaicEditorPanel::GetParameterFromID(const UUID64 paramId)
{
	for (auto& node : m_material->GetGraph().GetUnderlyingGraph().GetNodes())
	{
		for (auto& param : node.nodeData->GetInputParameters())
		{
			if (param.id == paramId)
			{
				return param;
			}
		}

		for (auto& param : node.nodeData->GetOutputParameters())
		{
			if (param.id == paramId)
			{
				return param;
			}
		}
	}

	static Mosaic::Parameter nullParam{ .id = 0 };
	return nullParam;
}

void MosaicEditorPanel::InitializeEditor()
{
	if (m_context.editorContext)
	{
		ed::DestroyEditor(m_context.editorContext);
	}

	ax::NodeEditor::Config cfg{};
	cfg.SettingsFile = nullptr;
	cfg.UserPointer = this;

	cfg.SaveSettings = [](const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer)
	{
		MosaicEditorPanel* editor = static_cast<MosaicEditorPanel*>(userPointer);
		return editor->SaveSettings(data);
	};

	cfg.LoadSettings = [](char* data, void* userPointer) -> size_t
	{
		MosaicEditorPanel* editor = static_cast<MosaicEditorPanel*>(userPointer);

		std::string graphContext;
		editor->LoadSettings(graphContext);

		if (data)
		{
			memcpy_s(data, graphContext.size(), graphContext.c_str(), graphContext.size());
		}

		return graphContext.size();
	};

	cfg.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
	{
		MosaicEditorPanel* editor = static_cast<MosaicEditorPanel*>(userPointer);
		return editor->SaveNodeSettings(nodeId.Get(), data);
	};

	cfg.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
	{
		MosaicEditorPanel* editor = static_cast<MosaicEditorPanel*>(userPointer);

		std::string graphContext;
		editor->LoadNodeSettings(nodeId.Get(), graphContext);

		if (data)
		{
			memcpy_s(data, graphContext.size(), graphContext.c_str(), graphContext.size());
		}

		return graphContext.size();
	};

	m_context.editorContext = ed::CreateEditor(&cfg);
	ed::SetCurrentEditor(m_context.editorContext);
	ed::EnableShortcuts(true);

	InitializeStyle(ed::GetStyle());
}

void MosaicEditorPanel::InitializeStyle(ax::NodeEditor::Style& editorStyle)
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

void MosaicEditorPanel::DrawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Create"))
			{
				std::filesystem::path path = FileSystem::SaveFileDialogue({{ "Mosaic Graph (*.vtmat)", "vtmat" }}, Volt::ProjectManager::GetAssetsDirectory());
				m_material = Volt::AssetManager::CreateAsset<Volt::Material>(path.parent_path(), path.stem().string());
				
				Volt::AssetManager::SaveAsset(m_material);
			}

			if (ImGui::MenuItem("Save") && m_material)
			{
				Volt::AssetManager::SaveAsset(m_material);
			}

			if (ImGui::MenuItem("Load"))
			{
				std::filesystem::path path = FileSystem::OpenFileDialogue({ { "Mosaic Graph (*.vtmat)", "vtmat" }}, Volt::ProjectManager::GetAssetsDirectory());
				m_material = Volt::AssetManager::GetAsset<Volt::Material>(path);
			}

			if (ImGui::MenuItem("Compile") && m_material)
			{
				m_material->Compile();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void MosaicEditorPanel::DrawEditor()
{
	ImGui::SetNextWindowDockID(m_mainDockID, ImGuiCond_Always);
	ImGui::SetNextWindowClass(GetWindowClass());

	ImGui::SetNextWindowSizeConstraints({ 100.f, 100.f }, { 0.f, 0.f });

	const std::string id = "Editor##mosaic";
	ImGui::Begin(id.c_str());

	ed::SetCurrentEditor(m_context.editorContext);

	if (ed::Begin("mosaic"))
	{
		DrawNodes();
		DrawLinks();

		if (!m_createNewNode)
		{
			if (ed::BeginCreate({ 1.f, 1.f, 1.f, 1.f }, 2.f))
			{
				OnBeginCreate();
			}
			else
			{
				m_newLinkPinId = 0;
			}

			ed::EndCreate();

			if (ed::BeginShortcut())
			{
				if (ed::AcceptCopy())
				{
					OnCopy();
				}

				if (ed::AcceptCut())
				{
				}

				if (ed::AcceptPaste())
				{
					OnPaste();
				}

				if (ed::AcceptDuplicate())
				{
				}
			}
			ed::EndShortcut();

			if (ed::BeginDelete())
			{
				OnBeginDelete();
			}
			ed::EndDelete();
		}

		ed::Suspend();
		if (ed::ShowNodeContextMenu(&m_context.contextNodeId))
		{

		}

		if (ed::ShowPinContextMenu(&m_context.contextPinId))
		{
		}

		if (ed::ShowLinkContextMenu(&m_context.contextLinkId))
		{
		}

		if (ed::ShowBackgroundContextMenu())
		{

		}

		DrawContextPopups();
		ed::Resume();
		ed::End();
		ed::SetCurrentEditor(nullptr);
	}

	ImGui::End();
}


void MosaicEditorPanel::DrawPanels()
{
	DrawNodesPanel();
}

void MosaicEditorPanel::DrawNodes()
{
	if (!m_material)
	{
		return;
	}

	ImTextureID textureId = nullptr;
	int32_t width = 0;
	int32_t height = 0;

	if (m_headerTexture && m_headerTexture->IsValid())
	{
		textureId = UI::GetTextureID(m_headerTexture);
		width = m_headerTexture->GetWidth();
		height = m_headerTexture->GetHeight();
	}

	utils::BlueprintNodeBuilder builder{ textureId, width, height };

	auto& graph = m_material->GetGraph().GetUnderlyingGraph();

	for (const auto& node : graph.GetNodes())
	{
		builder.Begin(ed::NodeId(node.id));

		auto nodeData = node.nodeData;

		const auto nodeColor = nodeData->GetColor();

		builder.Header(ImColor{ nodeColor.x, nodeColor.y, nodeColor.z, nodeColor.w });
		{
			ImGui::Spring(0.f);
			ImGui::TextUnformatted(nodeData->GetName().c_str());
			ImGui::Spring(1.f);

			constexpr float NODE_HEADER_HEIGHT = 18.f;
			ImGui::Dummy(ImVec2{ 0.f, NODE_HEADER_HEIGHT });

			ImGui::Spring(0.f);
			builder.EndHeader();
		}

		IONodeGraphEditorHelpers::BeginAttributes();

		for (auto& input : node.nodeData->GetInputParameters())
		{
			float alpha = ImGui::GetStyle().Alpha;
			ed::PinId linkStartPin = { static_cast<ed::PinId>(m_newLinkPinId) };
			ed::PinId inputPin = { static_cast<ed::PinId>(input.id)};

			const auto reason = CanLinkPins(linkStartPin.Get(), inputPin.Get());
			if (m_newLinkPinId && reason != IncompatiblePinReason::None && input.id != m_newLinkPinId)
			{
				alpha = alpha * (48.f / 255.f);
			}

			UI::ScopedStyleFloat alphaStyle{ ImGuiStyleVar_Alpha, alpha };
			builder.Input(ed::PinId(input.id));

			const bool connected = Utility::IsParameterLinked(m_material->GetGraph(), input.id);
			glm::vec4 color = Utility::GetColorFromTypeInfo(input.typeInfo);

			Utility::DrawPinIcon(input, connected, ImColor{ color.x, color.y, color.z, color.w }, (int32_t)(alpha * 255.f));

			ImGui::Spring(0.f);
			ImGui::TextUnformatted(input.name.c_str());

			if (!connected && input.typeInfo.baseType != Mosaic::ValueBaseType::Dynamic && input.showAttribute)
			{
				Utility::DrawAttribute(input);
			}

			ImGui::Spring(0.f);
			builder.EndInput();
		}

		for (auto& output : node.nodeData->GetOutputParameters())
		{
			float alpha = ImGui::GetStyle().Alpha;
			ed::PinId linkStartPin{ m_newLinkPinId };
			ed::PinId outputPin{ output.id };

			const auto reason = CanLinkPins(linkStartPin.Get(), outputPin.Get());
			if (m_newLinkPinId && reason != IncompatiblePinReason::None && output.id != m_newLinkPinId)
			{
				alpha = alpha * (48.f / 255.f);
			}

			UI::ScopedStyleFloat alphaStyle{ ImGuiStyleVar_Alpha, alpha };
			builder.Output(ed::PinId(output.id));

			const bool connected = Utility::IsParameterLinked(m_material->GetGraph(), output.id);
			ImGui::Spring(0.f);

			if (output.typeInfo.baseType != Mosaic::ValueBaseType::Dynamic && output.showAttribute)
			{
				Utility::DrawAttribute(output);
			}

			ImGui::TextUnformatted(output.name.c_str());
			ImGui::Spring(0.f);

			glm::vec4 color = Utility::GetColorFromTypeInfo(output.typeInfo);

			Utility::DrawPinIcon(output, connected, ImColor{ color.x, color.y, color.z, color.w }, (int32_t)(alpha * 255.f));

			builder.EndOutput();
		}

		IONodeGraphEditorHelpers::EndAttributes();

		node.nodeData->RenderCustomWidget();

		builder.End();
	}
}

void MosaicEditorPanel::DrawLinks()
{
	if (!m_material)
	{
		return;
	}

	const auto& graph = m_material->GetGraph().GetUnderlyingGraph();

	for (const auto& edge : graph.GetEdges())
	{
		const auto& startNode = graph.GetNodeFromID(edge.startNode);
		const auto& endNode = graph.GetNodeFromID(edge.endNode);

		const auto& startParam = startNode.nodeData->GetOutputParameter(edge.metaDataType->GetParameterOutputIndex());
		const auto& endParam = endNode.nodeData->GetInputParameter(edge.metaDataType->GetParameterInputIndex());

		ed::Link(ed::LinkId(edge.id), ed::PinId(startParam.id), ed::PinId(endParam.id));
	}
}

void MosaicEditorPanel::DrawNodesPanel()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	if (ImGui::Begin("Nodes", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		std::unordered_map<std::string, Vector<VoltGUID>> categorizedNodes;

		for (const auto& [guid, info] : Mosaic::NodeRegistry::GetRegistry())
		{
			categorizedNodes[info.category].emplace_back(guid);
		}

		UI::PushID();

		ImGui::BeginChild("Main", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			static std::string searchQuery;
			bool hasQuery;

			EditorUtils::SearchBar(searchQuery, hasQuery, false);

			UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
			ImGui::BeginChild("Scrollable", ImGui::GetContentRegionAvail());
			{
				for (const auto& [category, nodeGuids] : categorizedNodes)
				{
					// #TODO_Ivar: Implement search and pin type checking

					if (ImGui::TreeNodeEx(category.c_str()))
					{
						for (const auto& guid : nodeGuids)
						{
							const auto& nodeInfo = Mosaic::NodeRegistry::GetNodeInfo(guid);
							
							if (ImGui::MenuItem(nodeInfo.name.c_str()) && m_material)
							{
								m_material->GetGraph().AddNode(guid);
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

	}
	ImGui::End();
}

void MosaicEditorPanel::DrawContextPopups()
{
}

void MosaicEditorPanel::OnBeginCreate()
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
		auto& startParam = GetParameterFromID(startPinId.Get());
		auto& endParam = GetParameterFromID(endPinId.Get());

		m_newLinkPinId = startParam.id != 0 ? startParam.id : endParam.id;

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
		else if (ed::AcceptNewItem(ImColor{ 1.f, 1.f, 1.f }, 2.f))
		{
			auto& inputDirParam = startParam.direction == Mosaic::ParameterDirection::Input ? startParam : endParam;

			Utility::ClearLinksFromParameter(m_material->GetGraph(), inputDirParam.id);

			const auto startNode = Utility::GetNodeIdFromParameter(m_material->GetGraph(), startParam.id);
			const auto endNode = Utility::GetNodeIdFromParameter(m_material->GetGraph(), endParam.id);

			if (startNode != 0 && endNode != 0)
			{
				m_material->GetGraph().GetUnderlyingGraph().LinkNodes(startNode, endNode, CreateRef<Mosaic::MosaicEdge>(endParam.index, startParam.index));
			}
		}
	}
}

void MosaicEditorPanel::OnBeginDelete()
{
	ed::LinkId linkId;
	while (ed::QueryDeletedLink(&linkId))
	{
		if (ed::AcceptDeletedItem())
		{
			m_material->GetGraph().GetUnderlyingGraph().RemoveEdge(linkId.Get());
		}
	}

	ed::NodeId nodeId;
	while (ed::QueryDeletedNode(&nodeId))
	{
		if (ed::AcceptDeletedItem())
		{
			m_material->GetGraph().GetUnderlyingGraph().RemoveNode(nodeId.Get());
		}
	}
}

void MosaicEditorPanel::OnCopy()
{
}

void MosaicEditorPanel::OnPaste()
{
}
