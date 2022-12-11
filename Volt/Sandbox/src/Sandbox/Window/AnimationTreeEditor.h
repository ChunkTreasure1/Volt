#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include "../NodeEditor/AnimNode.h"


namespace ed = ax::NodeEditor;

static ed::EditorContext* Context = nullptr;

static std::vector<ed::EditorContext*> s_Contexts;
static std::vector<AnimNode>    s_Nodes;
static std::vector<AnimLink>    s_Links;
static std::vector<AnimationParameter> s_Parameters;

static int currentContext = 0;
static int s_NextId = 1;
static int GetNextId()
{
    return s_NextId++;
}

static const float          s_TouchTime = 1.0f;
static std::map<ed::NodeId, float, NodeIdLess> s_NodeTouchTime;

static ed::LinkId GetNextLinkId()
{
    return ed::LinkId(GetNextId());
}

static void TouchNode(ed::NodeId id)
{
    s_NodeTouchTime[id] = s_TouchTime;
}

static float GetTouchProgress(ed::NodeId id)
{
    auto it = s_NodeTouchTime.find(id);
    if (it != s_NodeTouchTime.end() && it->second > 0.0f)
        return (s_TouchTime - it->second) / s_TouchTime;
    else
        return 0.0f;
}

static void UpdateTouch()
{
    const auto deltaTime = ImGui::GetIO().DeltaTime;
    for (auto& entry : s_NodeTouchTime)
    {
        if (entry.second > 0.0f)
            entry.second -= deltaTime;
    }
}

static AnimNode* FindNode(ed::NodeId id)
{
    for (auto& node : s_Nodes)
        if (node.nodeID == id)
            return &node;

    return nullptr;
}

static AnimLink* FindLink(ed::LinkId id)
{
    for (auto& link : s_Links)
        if (link.linkID == id)
            return &link;

    return nullptr;
}

static Pin* FindPin(ed::PinId id)
{
    if (!id)
        return nullptr;

    for (auto& node : s_Nodes)
    {
        for (auto& pin : node.inputs)
            if (pin.pinID == id)
                return &pin;

        for (auto& pin : node.outputs)
            if (pin.pinID == id)
                return &pin;
    }

    return nullptr;
}

static AnimNode* FindNodeFromPin(ed::PinId id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.inputs)
			if (pin.pinID == id)
				return &node;

		for (auto& pin : node.outputs)
			if (pin.pinID == id)
				return &node;
	}

	return nullptr;
}

static AnimNode* FindStartNodeFromPin(ed::PinId id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.inputs)
			if (pin.pinID == id)
				return &node;
	}

	return nullptr;
}

static AnimNode* FindStartNodeFromPin(int id)
{
	if (id < 0)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.inputs)
			if (pin.id == id)
				return &node;
	}

	return nullptr;
}


static AnimNode* FindEndNodeFromPin(ed::PinId id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.outputs)
			if (pin.pinID == id)
				return &node;
	}

	return nullptr;
}

static bool IsPinLinked(ed::PinId id)
{
    if (!id)
        return false;

    for (auto& link : s_Links)
        if (link.startPin == id || link.endPin == id)
            return true;

    return false;
}

static std::vector<AnimationParameter>& GetParameters()
{
    s_Parameters;
}

static void AddParameter(const std::string& name)
{
    s_Parameters.emplace_back();
    s_Parameters.back().name = name;
    s_Parameters.back().value = 0.f;
}

//static bool CanCreateLink(Pin* a, Pin* b)
//{
//    if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
//        return false;
//
//    return true;
//}

class AnimationTreeEditor : public EditorWindow
{
public:
	AnimationTreeEditor();
	void UpdateMainContent() override;

private:

	AnimNode* SpawnTreeSequenceNode(int aID, std::string aNodeName, int animationID);

	void ShowAnimationPanel();
    void ShowPropertiesPanel();
    void ShowParameterPanel();
	void BuildNode(AnimNode* node);
	void BuildAllNodes()
	{
		for (auto& node : s_Nodes)
		{
			BuildNode(&node);
		}
	}	
    
    ed::Config myDefaultConfig;
    ed::EditorContext* currentContext;

	void Save();
	void SaveAs();
	void Load();

    Ref<Volt::AnimatedCharacter> myCurrentCharacter;

	std::vector<ax::NodeEditor::NodeId> mySelectedNodes;
	std::vector<ax::NodeEditor::LinkId> mySelectedLinks;
    std::vector<const char*> myBlendParams;
    std::vector<const char*> myBlendCompTypes = { "==", ">", "<" };

    AnimNode* mySelectedNode;
	AnimLink* mySelectedLink;



};



