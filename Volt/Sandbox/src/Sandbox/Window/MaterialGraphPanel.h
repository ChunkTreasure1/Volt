#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/NodeGraph/IONodeGraphEditor.h"
#include "Sandbox/NodeGraph/NodeGraphEditorBackend.h"

namespace Volt
{
	class MaterialGraphAsset;
}

struct MaterialGraphBackend : public NodeGraph::EditorBackend
{
	~MaterialGraphBackend() override = default;
};

class MaterialGraphPanel : public IONodeGraphEditor<GraphKey::GraphType::Material, MaterialGraphBackend>
{
public:
	MaterialGraphPanel(Ref<Volt::Scene>& currentScene);
	~MaterialGraphPanel() override;

	void OpenAsset(Ref<Volt::Asset> asset) override;

	bool SaveSettings(const std::string& data) override;
	size_t LoadSettings(std::string& data) override;

	bool SaveNodeSettings(const Volt::UUID nodeId, const std::string& data) override;
	size_t LoadNodeSettings(const Volt::UUID nodeId, std::string& data) override;

	inline static MaterialGraphPanel& Get() { return *myInstance; }

private:
	inline static MaterialGraphPanel* myInstance = nullptr;

	Ref<Volt::MaterialGraphAsset> myOpenMaterialGraph;
};
