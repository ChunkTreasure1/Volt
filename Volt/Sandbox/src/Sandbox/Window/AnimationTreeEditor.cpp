#include "sbpch.h"
#include "AnimationTreeEditor.h"
#include <yaml-cpp/yaml.h>
#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/FileSystem.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Animation/Animation.h>

#include <Windows.h>

#include <Volt/Utility/UIUtility.h>
#include <string>

namespace ed = ax::NodeEditor;

AnimationTreeEditor::AnimationTreeEditor()
	: EditorWindow("AnimationTree")
{
	myWindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	myNodePanelFlags = ImGuiWindowFlags_MenuBar;
	myIsOpen = false;

	currentContext = ed::CreateEditor(&myDefaultConfig);
	s_Contexts.push_back(currentContext);
	ed::SetCurrentEditor(currentContext);
}

void AnimationTreeEditor::UpdateMainContent()
{
	auto& io = ImGui::GetIO();

	ShowAnimationPanel();
	ShowPropertiesPanel();
	ShowParameterPanel();

	ImGui::SameLine();
	{
		static ImVec2 WindowSize = { 1024,1024 };
		ImGui::BeginChild("NODE EDITOR", WindowSize, true, 0);
		{
			ed::SetCurrentEditor(currentContext);
			ed::Begin("NodeEditor", ImVec2(0, 0));
			// Start drawing nodes.
			for (auto& node : s_Nodes)
			{
				const float rounding = 5.0f;
				const float padding = 5.0f;

				const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

				ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
				ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
				ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
				ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

				ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(64, 64, 64, 64));
				ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
				ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
				ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
				ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
				ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
				ed::PushStyleVar(ed::StyleVar_PinRadius, 10.0f);

				ed::BeginNode(node.nodeID);


				//Draw Inputs
				if (!node.inputs.empty())
				{
					auto& pin = node.inputs[0];
					ImGui::Dummy(ImVec2(0, padding));

					ed::PushStyleVar(ed::StyleVar_PinCorners, 12);

					ImGui::GetItemRectMin();

					ed::BeginPin(node.inputs[0].id, ed::PinKind::Input);
					ImGui::Text("TO");
					ed::PinPivotRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
					ed::PinRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
					ed::EndPin();
					ed::PopStyleVar();
				}
				ImGui::SameLine();
				ImGui::Text(node.name.c_str());
				ImGui::SameLine();
				ImGui::Text(std::to_string(node.animationIndex).c_str());
				
				//Draw Outputs
				if (!node.outputs.empty())
				{
					auto& pin = node.outputs[0];
					ImGui::Dummy(ImVec2(0, padding));
					ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
					ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
					ed::PushStyleVar(ed::StyleVar_PinCorners, 12);

					ed::BeginPin(pin.id, ed::PinKind::Output);
					ImGui::Text("FROM");
					ed::EndPin();
					ed::PopStyleVar(3);
				}

				ed::EndNode();
				ed::PopStyleVar(7);
				ed::PopStyleColor(4);


			}

			for (auto& link : s_Links)
				ed::Link(link.linkID, link.startPin, link.endPin, link.color, 2.0f);


			static Pin* newLinkPin = nullptr;
			if (ed::BeginCreate(ImColor(255, 255, 0), 2.0f))
			{
				auto showLabel = [](const char* label, ImColor color)
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
					auto size = ImGui::CalcTextSize(label);

					auto padding = ImGui::GetStyle().FramePadding;
					auto spacing = ImGui::GetStyle().ItemSpacing;

					ImGui::SetCursorPos(ImGui::GetCursorPos()/* + ImVec2(spacing.x, -spacing.y)*/);

					//auto rectMin = ImGui::GetCursorScreenPos() - padding;
					//auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

					//auto rectMin = ImGui::GetCursorScreenPos() - 20;
					//auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

					auto drawList = ImGui::GetWindowDrawList();
					drawList->AddRectFilled(ImVec2(128, 128), ImVec2(128, 128), color, size.y * 0.15f);
					ImGui::TextUnformatted(label);
				};

				ed::PinId startPinId = 0, endPinId = 0;
				if (ed::QueryNewLink(&startPinId, &endPinId))
				{
					auto startPin = FindPin(startPinId);
					auto endPin = FindPin(endPinId);

					newLinkPin = startPin ? startPin : endPin;

					if (startPin->mode == PinMode::Input)
					{
						std::swap(startPin, endPin);
						std::swap(startPinId, endPinId);
					}

					if (startPin && endPin)
					{
						if (endPin == startPin)
						{
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (endPin->mode == startPin->mode)
						{
							showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (endPin->node == startPin->node)
						{
							showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
						}
						else if (endPin->type != startPin->type)
						{
							showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
						}
						else
						{
							showLabel("+ Create Link", ImColor(32, 45, 32, 180));
							if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
							{
								//s_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
								s_Links.emplace_back(AnimLink(GetNextId(), startPin->id, endPin->id));
								//s_Links.back().Color = GetIconColor(startPin->Type);
								s_Links.back().color = ImColor(0, 0, 255);
							}
						}
					}
				}
			}

			ed::EndCreate();

			if (ed::BeginDelete())
			{
				ed::LinkId linkId = 0;
				while (ed::QueryDeletedLink((&linkId)))
				{
					if (ed::AcceptDeletedItem())
					{
						auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link.linkID == linkId; });
						if (id != s_Links.end())
						{
							s_Links.erase(id);
						}

					}
				}

				ed::NodeId nodeId = 0;
				while (ed::QueryDeletedNode(&nodeId))
				{
					if (ed::AcceptDeletedItem())
					{
						auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.nodeID == nodeId; });
						if (id != s_Nodes.end())
						{
							s_Nodes.erase(id);
						}
					}
				}
			}
			ed::EndDelete();

			auto openPopupPosition = ImGui::GetMousePos();
			ed::Suspend();
			if (ed::ShowBackgroundContextMenu())
			{
				ImGui::OpenPopup("Create New Node");
			}
			ed::Resume();

			ed::Suspend();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
			if (ImGui::BeginPopup("Create New Node"))
			{
				auto newNodePostion = openPopupPosition;

				AnimNode* node = nullptr;

				if (myCurrentCharacter)
				{
					for (const auto& [index, anim] : myCurrentCharacter->GetAnimations())
					{
						std::string animName;
						if (anim && anim->IsValid())
						{
							animName = anim->path.stem().string();
						}
						else
						{
							"Null";
						}

						if (ImGui::MenuItem(animName.c_str()))
							node = SpawnTreeSequenceNode(GetNextId(), animName.c_str(), index);
					}

					if (node)
					{
						BuildAllNodes();
						ed::SetNodePosition(node->nodeID, newNodePostion);
					}

				}

				ImGui::EndPopup();
			}

			ImGui::PopStyleVar();
			ed::Resume();

			ed::End();
			
		}
		ImGui::EndChild();
	}

	mySelectedNodes.resize(ed::GetSelectedObjectCount());
	mySelectedLinks.resize(ed::GetSelectedObjectCount());

	int nodeCount = ed::GetSelectedNodes(mySelectedNodes.data(), static_cast<int>(mySelectedNodes.size()));
	int linkCount = ed::GetSelectedLinks(mySelectedLinks.data(), static_cast<int>(mySelectedLinks.size()));

	mySelectedNodes.resize(nodeCount);
	mySelectedLinks.resize(linkCount);

	ed::SetCurrentEditor(nullptr);

}

AnimNode* AnimationTreeEditor::SpawnTreeSequenceNode(int aID, std::string aNodeName, int animationID)
{
	s_Nodes.emplace_back(aID, aNodeName.c_str(), animationID);
	s_Nodes.back().Type = NodeType::Tree;
	s_Nodes.back().inputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

void AnimationTreeEditor::ShowAnimationPanel()
{
	auto& io = ImGui::GetIO();

	static bool isOpen;
	if (ImGui::Begin("Nodes", &isOpen, myNodePanelFlags))
	{
		if (ImGui::BeginMenuBar())
		{
			static bool save = false;
			static bool load = false;
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Open Character"))
				{
					const std::filesystem::path path = FileSystem::GetPathRelativeToBaseFolder(FileSystem::OpenFile("Animated Character (*.vtchr)\0*.vtchr\0"));
					if (!path.empty() && FileSystem::Exists(path))
					{
						myCurrentCharacter = Volt::AssetManager::GetAsset<Volt::AnimatedCharacter>(path);
					}
				}

				if (ImGui::MenuItem("Save"))
				{
					Save();
				}
				if (ImGui::MenuItem("Load"))
				{
					Load();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (!myCurrentCharacter)
		{
			ImGui::End();
			return;
		}

		ImGui::Text("CurrentFile : ");
		ImGui::SameLine();
		std::string ChrName = myCurrentCharacter->path.filename().string();
		ImGui::Text(ChrName.c_str());

		if (ImGui::BeginTable("AnimTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthStretch, 0.3f);
			ImGui::TableSetupColumn("Animation", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (const auto& [index, anim] : myCurrentCharacter->GetAnimations())
			{
				ImGui::TableNextColumn();
				ImGui::Text("%d", index);

				ImGui::TableNextColumn();

				std::string animName;
				if (anim && anim->IsValid())
				{
					animName = anim->path.stem().string();
				}
				else
				{
					"Null";
				}

				std::string popupName = "animPopup" + std::to_string(index);

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				UI::InputText("", animName, ImGuiInputTextFlags_ReadOnly);
			}

			ImGui::EndTable();
		}

		
	}

		ImGui::End();
}

void AnimationTreeEditor::ShowPropertiesPanel()
{
	ImGui::Begin("PropertiesPanel");

	if (mySelectedNodes.size() == 1)
	{
		mySelectedNode = FindNode(mySelectedNodes[0]);

		bool open = ImGui::BeginTable("", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable);

		if (open)
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.3f);
			ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch);
			ImGui::EndTable();
		}
	}
	else if (mySelectedLinks.size() == 1)
	{
		mySelectedLink = FindLink(mySelectedLinks[0]);

		bool open = ImGui::BeginTable("", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable);

		if (open)
		{
			ImGui::TableSetupColumn("Exit Time", ImGuiTableColumnFlags_WidthStretch, 0.3f);
			ImGui::TableSetupColumn("Blend Time", ImGuiTableColumnFlags_WidthStretch);
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Add Requirement"))
		{
			mySelectedLink->blendRequirements.emplace_back();
		}

		myBlendParams.clear();
		myBlendParams.emplace_back("None");

		for (const auto& param : s_Parameters)
		{
			myBlendParams.emplace_back(param.name.c_str());
		}

		uint32_t index = 0;

		for (auto& blendReq : mySelectedLink->blendRequirements)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });
			float width = ImGui::GetContentRegionAvail().x / 3.f - 22.f / 3.f;

			int currentlySelected = 0;

			if (blendReq.parameter)
			{
				uint32_t paramIndex = 1;
				for (const auto& param : s_Parameters)
				{
					if (blendReq.parameter == &param)
					{
						currentlySelected = paramIndex;
						break;
					}
					paramIndex++;
				}
			}

			std::string paramId = "##param" + std::to_string(index);
			ImGui::PushItemWidth(width);
			if (ImGui::Combo(paramId.c_str(), &currentlySelected, myBlendParams.data(), (int)myBlendParams.size()))
			{
				if (currentlySelected == 0)
				{
					blendReq.parameter = nullptr;
				}
				else
				{
					blendReq.parameter = &const_cast<AnimationParameter&>(s_Parameters[currentlySelected - 1]);
				}
			}

			ImGui::PopItemWidth();
			ImGui::SameLine();

			std::string typeId = "##type" + std::to_string(index);
			ImGui::PushItemWidth(width);
			ImGui::Combo(typeId.c_str(), (int*)&blendReq.comparisonType, myBlendCompTypes.data(), (int)myBlendCompTypes.size());

			ImGui::PopItemWidth();
			ImGui::SameLine();

			std::string valueId = "##value" + std::to_string(index);
			ImGui::PushItemWidth(width);
			ImGui::DragFloat(valueId.c_str(), &blendReq.value);

			ImGui::PopItemWidth();

			ImGui::SameLine();

			std::string remId = "-##" + std::to_string(index);
			if (ImGui::Button(remId.c_str(), { 22.f, 22.f }))
			{
				mySelectedLink->blendRequirements.erase(mySelectedLink->blendRequirements.begin() + index);
				ImGui::PopStyleVar();
				break;
			}

			ImGui::PopStyleVar();
			index++;
		}
	}

	ImGui::End();

}

void AnimationTreeEditor::ShowParameterPanel()
{
	ImGui::Begin("Animation Parameters");

	if (ImGui::Button("Add"))
	{
		AddParameter("New Parameter");
	}

	ImGui::Separator();

	auto& params = const_cast<std::vector<AnimationParameter>&>(s_Parameters);

	uint32_t index = 0;
	for (auto& param : params)
	{
		ImGui::TextUnformatted("Param");
		ImGui::SameLine();

		auto width = ImGui::GetContentRegionAvail().x / 2.f - 11.f;

		ImGui::PushItemWidth(width);
		std::string id = "##" + std::to_string(index);
		ImGui::InputTextString(id.c_str(), &param.name);
		ImGui::PopItemWidth();

		ImGui::SameLine();

		ImGui::PushItemWidth(width);
		std::string inputId = "##input" + std::to_string(index);
		ImGui::DragFloat(inputId.c_str(), &param.value, 1.f);
		ImGui::PopItemWidth();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });
		ImGui::SameLine();

		std::string remId = "-##" + std::to_string(index);
		if (ImGui::Button(remId.c_str(), { 22.f, 22.f }))
		{
			params.erase(params.begin() + index);
			break;
		}

		ImGui::PopStyleVar();

		index++;
	}
	ImGui::End();

}

void AnimationTreeEditor::BuildNode(AnimNode* node)
{
	for (auto& input : node->inputs)
	{
		input.node = node;
		input.mode = PinMode::Input;
	}

	for (auto& output : node->outputs)
	{
		output.node = node;
		output.mode = PinMode::Output;
	}
}

void AnimationTreeEditor::Save()
{
	myDefaultConfig.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
	{
		auto node = FindNode(nodeId);
		if (!node)
			return false;

		node->State.assign(data, size);

		return true;
	};

	YAML::Emitter configOut;
	//Save Nodes

	using namespace Volt;

	configOut << YAML::BeginMap;
	VT_SERIALIZE_PROPERTY_STRING("CHRPath", myCurrentCharacter->path, configOut);
	configOut << YAML::Key << "Nodes" << YAML::Value;
	{
		configOut << YAML::BeginMap;
		int nodeNumber = 0;
		for (auto& aNode : s_Nodes)
		{
			configOut << YAML::Key << nodeNumber << YAML::Value;
			{
				configOut << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY_STRING("Name", aNode.name, configOut);
				VT_SERIALIZE_PROPERTY(ID, aNode.id, configOut);
				VT_SERIALIZE_PROPERTY(AnimationIndex, aNode.animationIndex, configOut);

				VT_SERIALIZE_PROPERTY(InputPin, aNode.inputs[0].id, configOut);
				VT_SERIALIZE_PROPERTY(InputPinType, (int)aNode.inputs[0].type, configOut);

				VT_SERIALIZE_PROPERTY(OutputPin, aNode.outputs[0].id, configOut);
				VT_SERIALIZE_PROPERTY(OutputPinType, (int)aNode.inputs[0].type, configOut);

				configOut << YAML::EndMap;
			}
			nodeNumber++;
		}
		configOut << YAML::EndMap;
	}

	configOut << YAML::Key << "Links" << YAML::Value;
	{
		configOut << YAML::BeginMap;
		int linkNumber = 0;
		for (auto& aLink : s_Links)
		{
			configOut << YAML::Key << linkNumber << YAML::Value;
			{
				configOut << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(linkID, aLink.id, configOut);
				VT_SERIALIZE_PROPERTY(startPinID, aLink.startPinID, configOut);
				VT_SERIALIZE_PROPERTY(endPinID, aLink.endPinID, configOut);
				configOut << YAML::EndMap;
			}
			linkNumber++;
		}
		configOut << YAML::EndMap;
	}
	configOut << YAML::EndMap;

	std::wstring outputFolder = L"Editor/AnimationTree";
	if (CreateDirectory(outputFolder.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		std::ofstream fout("Editor/AnimationTree/Test.treeconfig");
		fout << configOut.c_str();
		fout.close();
	}

	YAML::Emitter treeOut;
	treeOut << YAML::BeginMap;
	VT_SERIALIZE_PROPERTY_STRING("CHRPath", myCurrentCharacter->handle, treeOut);
	treeOut << YAML::Key << "Nodes" << YAML::Value;
	{
		treeOut << YAML::BeginMap;
		int nodeNumber = 0;
		for (auto& aNode : s_Nodes)
		{
			treeOut << YAML::Key << nodeNumber << YAML::Value;
			{
				treeOut << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY_STRING("Name", aNode.name, treeOut);
				VT_SERIALIZE_PROPERTY_STRING("NodeID", aNode.id, treeOut);
				VT_SERIALIZE_PROPERTY(AnimationIndex, aNode.animationIndex, treeOut);
				std::vector<int> nodePaths;
				for (auto& aLink : s_Links)
				{
					int LinksNodeID = FindNodeFromPin(aLink.endPinID)->id;
					if (LinksNodeID == aNode.id)
					{
						nodePaths.push_back(FindNodeFromPin(aLink.startPinID)->id);
					}
				}

				treeOut << YAML::Key << "NodePaths" << YAML::Flow << nodePaths;

				treeOut << YAML::EndMap;
			}
			nodeNumber++;
		}
		treeOut << YAML::EndMap;
	}

	treeOut << YAML::Key << "Links" << YAML::Value;
	{
		treeOut << YAML::BeginMap;
		int linkNumber = 0;
		for (auto& aLink : s_Links)
		{
			treeOut << YAML::Key << linkNumber << YAML::Value;
			{
				treeOut << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(LinkID, aLink.id, treeOut);
				int LinksStartNodeID = FindStartNodeFromPin(aLink.startPin)->id;
				VT_SERIALIZE_PROPERTY(LinksStartNodeID, aLink.id, treeOut);
				int LinksEndNodeID = FindEndNodeFromPin(aLink.endPin)->id;
				VT_SERIALIZE_PROPERTY(LinksEndNodeID, aLink.id, treeOut);
				VT_SERIALIZE_PROPERTY(HasExitTime, aLink.hasExitTime, treeOut);
				VT_SERIALIZE_PROPERTY(blendTime, aLink.blendTime, treeOut);
				
				int ReqNumber = 0;
				for (auto& blendReq : aLink.blendRequirements)
				{
					treeOut << YAML::Key << ReqNumber << YAML::Value;
					{
						treeOut << YAML::BeginMap;
						VT_SERIALIZE_PROPERTY_STRING("parameterName", blendReq.parameter->name, treeOut);
						VT_SERIALIZE_PROPERTY(comparisonType, (int)blendReq.comparisonType, treeOut);
						VT_SERIALIZE_PROPERTY(parameterValue, blendReq.value, treeOut);
						treeOut << YAML::EndMap;
					}
					ReqNumber++;
				}

				treeOut << YAML::EndMap;
			}
			linkNumber++;
		}
		treeOut << YAML::EndMap;
	}

	treeOut << YAML::Key << "Parameters" << YAML::Value;
	{
		treeOut << YAML::BeginMap;
		int parameterNumber = 0;
		for (auto& aParameter : s_Parameters)
		{
			treeOut << YAML::Key << parameterNumber << YAML::Value;
			{
				treeOut << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY_STRING("parameterName", aParameter.name, treeOut);
				VT_SERIALIZE_PROPERTY(value, aParameter.value, treeOut);
				treeOut << YAML::EndMap;
			}
			parameterNumber++;
		}
		treeOut << YAML::EndMap;
	}

	treeOut << YAML::EndMap;

	outputFolder = L"Editor/AnimationTree";
	if (CreateDirectory(outputFolder.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		std::ofstream fout("Editor/AnimationTree/Testie.animtree");
		fout << treeOut.c_str();
		fout.close();
	}

}

void AnimationTreeEditor::SaveAs()
{

}

void AnimationTreeEditor::Load()
{
	//Reset AnimationTree
	s_NextId = 1;
	s_Nodes.clear();
	s_Links.clear();

	//Load YAML File
	std::ifstream file("Editor/AnimationTree/Test.treeconfig");
	std::stringstream sstream;
	sstream << file.rdbuf();

	YAML::Node root = YAML::Load(sstream.str());
	YAML::Node Nodes = root["Nodes"];

	std::string aName = "";

	if (Nodes)
	{
		for (int currentNode = 0; currentNode < Nodes.size(); currentNode++) 
		{
			aName = Nodes[currentNode]["Name"].as<std::string>();
			int animationID = Nodes[currentNode]["AnimationIndex"].as<int>();

			AnimNode* node;
			node = SpawnTreeSequenceNode(GetNextId(), aName, animationID);
		}
	}

	BuildAllNodes();
	int startPinID = -1, endPinType = -1;
	YAML::Node Links = root["Links"];
	if (Links) 
	{
		int a = (int32_t)Links.size();

		for (int currentLink = 0; currentLink < Links.size(); currentLink++)
		{
			startPinID = Links[currentLink]["startPinID"].as<int>();
			endPinType = Links[currentLink]["endPinID"].as<int>();
			s_Links.push_back(AnimLink(GetNextId(), startPinID, endPinType));
			s_Links.back().color = ImColor(0, 0, 255);
		}
	}

	file.close();

	//Set Up Config file
	ed::Config aConfig("Editor/AnimationTree/NodeEditor.json");

	myDefaultConfig.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
	{
		auto node = FindNode(nodeId);
		if (!node)
			return 0;

		if (data != nullptr)
			memcpy(data, node->State.data(), node->State.size());
		return node->State.size();
	};
}
