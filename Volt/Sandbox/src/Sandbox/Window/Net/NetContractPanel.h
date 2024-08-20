#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include <Volt/Net/SceneInteraction/NetContract.h>
#include <unordered_set>

class NetContractPanel : public EditorWindow
{
public:
	NetContractPanel();
	~NetContractPanel();

	void UpdateMainContent() override;

private:
	void DrawPanel();
	void DrawActions();
	void DrawCreateContract();

	void DrawCalls(Ref<Volt::NetContract> in_contract);
	void DrawComponentRules(Ref<Volt::NetContract> in_contract);
	void DrawChildRules(Ref<Volt::NetContract> in_contract);
	void DrawChild(Ref<Volt::NetContract> in_contract, Volt::EntityID in_id);
	void RuleEntry(const std::string& in_name, Volt::NetRule& in_rules, const std::string& in_id);
	void DrawRuleSet(Ref<Volt::NetContract> in_contract, Volt::EntityID in_id);

	bool BlockStandardComponents(const std::string& in_name);
	std::unordered_set<std::string> m_blocked;
	Volt::AssetHandle m_handle = Volt::AssetHandle(0);
	Ref<Volt::Scene> m_scene;
	Volt::EntityID m_entityId;
};
