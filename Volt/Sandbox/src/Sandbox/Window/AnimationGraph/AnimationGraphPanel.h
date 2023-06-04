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
	Volt::UUID fromId;
	Volt::UUID toId;
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

	bool SaveNodeSettings(const Volt::UUID nodeId, const std::string& data)  override;
	size_t LoadNodeSettings(const Volt::UUID nodeId, std::string& data)  override;

protected:
	void DrawMenuBar() override;
	void DrawPanels() override;
	void DrawNodes() override;
	void DrawLinks() override;
	void DrawNodesPanel() override;

	void OnBeginCreate() override;
	void OnDeleteLink(const Volt::UUID id) override;
	void OnDeleteNode(const Volt::UUID id) override;

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

	const std::vector<Volt::UUID> GetSelectedStates() const;
	const std::vector<Volt::UUID> GetSelectedTransitions() const;

	inline const GraphDepthEntry& GetLastEntry() const { return myGraphDepth.back(); }

	NewAnimationGraphData myNewAnimGraphData{};
	std::vector<GraphDepthEntry> myGraphDepth;
	Ref<Volt::AnimationGraphAsset> myCurrentAsset;

	bool myShouldReconstruct = false;
};
