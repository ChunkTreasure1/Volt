#pragma once
#include "../../NodeGraph/NodeGraphEditor.h"
#include <Volt/BehaviorTree/BehaviorTree.hpp>

#include <imgui_internal.h>
#include <builders.h>
#include <typeindex>

namespace NodeGraph
{
	struct Node;
}


enum class ePinType
{
	INPUT,
	OUTPUT
};

struct OwnedPin
{
	UUID64 pinUUID = 0;
	UUID64 nodeUUID = 0;
	ePinType type = ePinType::INPUT;
};

class BehaviorEditor : public NodeGraph::Editor
{
public:
	BehaviorEditor(const std::string& title, const std::string& context);
	~BehaviorEditor() override;

	void OpenAsset(Ref<Volt::Asset> asset) override;

	template <class _T>
	UUID64 ConstructNewNode();
	void AddNodeToBackend(const UUID64& in_nodeID);

	void DrawNodes() override;
	void DrawPanels() override;
	void DrawContextPopups() override;

	void SortOutput();
	bool CanLink(OwnedPin in_pin1, OwnedPin in_pin2);

	void NodeDraw(const NodeGraph::Node& n);
	OwnedPin FindPin(ax::NodeEditor::PinId id);

	bool SaveSettings(const std::string& data)  override;
	size_t LoadSettings(std::string& data)  override;

	bool SaveNodeSettings(const UUID64 nodeId, const std::string& data)  override;
	size_t LoadNodeSettings(const UUID64 nodeId, std::string& data) override;
protected:
	Volt::BehaviorTree::Decorator* m_decPtr{ nullptr };
	Volt::Entity myTreeEntity = Volt::Entity::Null();
	Ref<Volt::Scene> m_scene;

	Ref<Volt::BehaviorTree::Tree> myBehaviourTree;
	Volt::AssetHandle m_currentHandle;
};

template<class _T>
inline UUID64 BehaviorEditor::ConstructNewNode()
{
	auto nNode = myBehaviourTree->CreateNode<_T>();
	AddNodeToBackend(nNode);
	return nNode;
}


