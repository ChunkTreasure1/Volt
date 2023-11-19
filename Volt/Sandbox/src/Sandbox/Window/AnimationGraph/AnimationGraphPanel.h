#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include "Sandbox/NodeGraph/IONodeGraphEditor.h"
#include "Sandbox/NodeGraph/NodeGraphEditorBackend.h"

#include "Sandbox/Utility/EditorUtilities.h"

#include <imgui_node_editor.h>

namespace Volt
{
	class Texture2D;
	class AnimationStateMachine;
	struct AnimationState;

	namespace AnimationGraph
	{
		class Graph;
		struct Node;
		struct Pin;
		struct Link;
	}
}

struct LinkPair
{
	UUID64 fromId;
	UUID64 toId;
};

inline bool operator==(const LinkPair& lhs, const LinkPair& rhs)
{
	return lhs.fromId == rhs.fromId && lhs.toId == rhs.toId;
}

namespace std
{
	template <>
	struct hash<LinkPair>
	{
		std::size_t operator()(const LinkPair& k) const
		{
			return k.fromId + k.toId;
		}
	};
}

struct AnimationBackend final : public NodeGraph::EditorBackend
{
	~AnimationBackend() override = default;
	const Volt::AssetType GetSupportedAssetTypes() override { return Volt::AssetType::Animation | Volt::AssetType::BlendSpace; }
};

class AnimationGraphPanel final : public IONodeGraphEditor<GraphKey::GraphType::Animation, AnimationBackend>
{
public:
	AnimationGraphPanel(Ref<Volt::Scene>& currentScene);
	~AnimationGraphPanel() override;

	void OpenAsset(Ref<Volt::Asset> asset) override;

	bool SaveSettings(const std::string& data)  override;
	size_t LoadSettings(std::string& data)  override;

	bool SaveNodeSettings(const UUID64 nodeId, const std::string& data)  override;
	size_t LoadNodeSettings(const UUID64 nodeId, std::string& data)  override;

protected:
	void DrawMenuBar() override;
	void DrawPanels() override;
	void DrawNodes() override;
	void DrawLinks() override;
	void DrawNodesPanel() override;

	void OnBeginCreate() override;
	void OnDeleteLink(const UUID64 id) override;
	void OnDeleteNode(const UUID64 id) override;

private:
	enum class EditorType
	{
		Graph,
		StateMachine
	};

	struct GraphDepthEntry
	{
		Ref<GraphKey::Graph> graph;
		Ref<Volt::AnimationStateMachine> stateMachine;

		std::string name = "Empty";
		EditorType editorType = EditorType::Graph;
	};

	void DrawPropertiesPanel();
	void DrawGraphDepthBar();
	void DrawStateMachineNodes();
	void OnBeginCreateStateMachine();

	void DrawGraphProperties();
	void DrawStateMachineProperties();

	void DrawLayeredBlendPerBoneProperties(Ref<GraphKey::Node> node);

	const std::vector<UUID64> GetSelectedStates() const;
	const std::vector<UUID64> GetSelectedTransitions() const;

	inline const GraphDepthEntry& GetLastEntry() const { return myGraphDepth.back(); }

	NewAnimationGraphData myNewAnimGraphData{};
	std::vector<GraphDepthEntry> myGraphDepth;
	Ref<Volt::AnimationGraphAsset> myCurrentAsset;

	bool myShouldReconstruct = false;
};
