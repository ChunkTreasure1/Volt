#pragma once

#include <imgui_node_editor.h>

#include <string>
#include <vector>

namespace ed = ax::NodeEditor;

enum class PinType
{
	Flow,
	Bool,
	Int,
	Float,
	String,
	Object,
	Function,
	Delegate
};

enum class PinMode
{
	Output,
	Input
};

enum class NodeType
{
	Blueprint,
	Simple,
	Tree,
	Comment,
	Houdini
};

struct AnimNode;

struct Pin
{
	ed::PinId pinID;
	int id;
	::AnimNode* node;
	std::string name;
	PinType type;
	PinMode mode;

	Pin(int aID, const char* aName, PinType aType)
		: pinID(aID), id(aID), node(nullptr), name(aName), type(aType), mode(PinMode::Input)
	{

	}
};

struct AnimNode
{
	ed::NodeId nodeID;
	int id;
	std::string name;
	int animationIndex;

	std::vector<Pin> inputs;
	std::vector<Pin> outputs;
	ImColor color;
	NodeType Type;
	ImVec2 size;


	std::string State;
	std::string SavedState;

	AnimNode(int aID, const char* aName, int aAnimationIndex, ImColor aColor = ImColor(255, 255, 255)) :
		nodeID(aID), id(aID), name(aName), animationIndex(aAnimationIndex), color(aColor), Type(NodeType::Tree), size(512, 512)
	{
	}
};

enum class ComparisonType
{
	Equal,
	Greater,
	Less
};

struct AnimationParameter
{
	std::string name;
	float value;
};

struct BlendRequirement
{
	ComparisonType comparisonType = ComparisonType::Equal;
	AnimationParameter* parameter = nullptr;
	float value = 0.f;
};

struct AnimLink
{
	ed::LinkId linkID = 0;
	int id = 0;

	ed::PinId startPin = 0;
	int startPinID = 0;
	ed::PinId endPin = 0;
	int endPinID = 0;

	bool hasExitTime = false;
	float blendTime = 0.1f;
	std::vector<BlendRequirement> blendRequirements;

	ImColor color;

	//Link(ed::LinkId aId, ed::PinId aStartId, ed::PinId aEndId)
	//	: linkID(aId), startPin(aStartId), endPin(aEndId)
	//{
	//}

	AnimLink(int aId, int aStartId, int aEndId)
		: linkID(aId), id(aId), startPin(aStartId), startPinID(aStartId), endPin(aEndId), endPinID(aEndId)
	{
	}

	AnimLink(ed::LinkId aId, int aStartId, int aEndId)
		: linkID(aId), startPin(aStartId), startPinID(aStartId), endPin(aEndId), endPinID(aEndId)
	{
	}
};

struct NodeIdLess
{
	bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
	{
		return lhs.AsPointer() < rhs.AsPointer();
	}
};



