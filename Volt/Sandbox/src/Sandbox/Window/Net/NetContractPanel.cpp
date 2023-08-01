#include "sbpch.h"
#include "NetContractPanel.h"

#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Asset/Prefab.h>
#include <Volt/Net/SceneInteraction/NetActorComponent.h>

#include <Wire/Registry.h>
#include <random>

NetContractPanel::NetContractPanel()
	: EditorWindow("NetContract Panel")
{
	m_scene = Volt::Scene::CreateDefaultScene("NetContract scene", false);
	m_blocked = {
		"VisualScriptingComponent",
		"TagComponent",
		"TransformComponent",
		"EntityDataComponent",
		"RelationshipComponent",
		"PrefabComponent",
		"NetActorComponent"
	};
}

NetContractPanel::~NetContractPanel()
{
}

void NetContractPanel::OnEvent(Volt::Event& e)
{
}

void NetContractPanel::UpdateMainContent()
{
	DrawPanel();
}

void NetContractPanel::DrawPanel()
{
	DrawActions();
	if (m_handle == 0) return;

	auto contract = Volt::NetContractContainer::GetContract(m_handle);
	if (!contract)
	{
		DrawCreateContract();
		return;
	}

	ImGui::BeginChild("Properties##NetContractContainer", { 0, 0 }, true);
	{
		DrawCalls(contract);
		ImGui::Separator();
		if (Volt::Entity(m_entityId, m_scene.get()).HasComponent<Volt::NetActorComponent>())
		{
			ImGui::TextColored(ImColor(0, 255, 0), "NetActorComponent");
		}
		else
		{
			ImGui::TextColored(ImColor(255, 0, 0), "NetActorComponent");
		}
		ImGui::Separator();
		DrawComponentRules(contract);
		DrawChildRules(contract);
	}ImGui::EndChild();
}

void NetContractPanel::DrawActions()
{
    // #nexus_todo: fix asset-> path stuff
	/*if (ImGui::Button("Save"))
	{
		if (!FileSystem::IsWriteable(Volt::ProjectManager::GetDirectory() / Volt::NetContractContainer::GetContract(m_handle)->path))
		{
			UI::Notify(NotificationType::Error, "NetContract save Failed", "Make sure file is writable");
		}
		else if (m_entityId != 0 && m_handle != 0)
		{
			UI::Notify(NotificationType::Success, "NetContract Saved ", "");
			auto contract = Volt::NetContractContainer::GetContract(m_handle);
			if (!std::filesystem::exists(Volt::AssetManager::GetContextPath(contract->path) / "Assets/Networking/Contracts"))
			{
				std::filesystem::create_directory(Volt::AssetManager::GetContextPath(contract->path) / "Assets/Networking/Contracts");
			}
			Volt::AssetManager::Get().SaveAsset(contract);
		}
		else
		{
			UI::Notify(NotificationType::Error, "NetContract save Failed", "Invalid preset");
		}
	}*/
	ImGui::SameLine();
	if (ImGui::Button("Refresh"))
	{
		if (m_entityId != 0 && m_handle != 0)
		{
			Volt::NetContractContainer::Clear();
			Volt::NetContractContainer::Load();
			m_scene->RemoveEntity(Volt::Entity(m_entityId, m_scene.get()));
			m_entityId = Volt::AssetManager::GetAsset<Volt::Prefab>(m_handle)->Instantiate(m_scene.get());
		}
	}
	ImGui::SameLine();
	if (EditorUtils::Property("Prefab:", m_handle, Volt::AssetType::Prefab))
	{
		if (m_entityId != 0)
		{
			m_scene->RemoveEntity(Volt::Entity(m_entityId, m_scene.get()));
			Volt::NetContractContainer::Clear();
			Volt::NetContractContainer::Load();
		}
		if (m_handle != 0)
		{
			m_entityId = Volt::AssetManager::GetAsset<Volt::Prefab>(m_handle)->Instantiate(m_scene.get());
			Volt::NetContractContainer::Clear();
			Volt::NetContractContainer::Load();
		}
	}
}

void NetContractPanel::DrawCalls(Ref<Volt::NetContract> in_contract)
{
	// Manage Calls
	static int addEvent = 0;
	auto& enumData = Wire::ComponentRegistry::EnumData();
	if (enumData.find("eNetEvent") != enumData.end())
	{
		UI::ComboProperty("Manage Calls", addEvent, enumData.at("eNetEvent"));
	}
	float halfSize = ImGui::GetContentRegionAvail().x * 0.5f;
	if (ImGui::Button("+##NetContractAddEvent", { halfSize , 0 }))
	{
		if (addEvent != 0 && !in_contract->calls.contains(static_cast<Volt::eNetEvent>(addEvent)))
		{
			in_contract->calls.insert({ static_cast<Volt::eNetEvent>(addEvent), "" });
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("-##NetContractRemoveEvent", { halfSize , 0 }))
	{
		if (addEvent != 0 && in_contract->calls.contains(static_cast<Volt::eNetEvent>(addEvent)))
		{
			in_contract->calls.erase(static_cast<Volt::eNetEvent>(addEvent));
		}
	}

	// Calls
	if (UI::BeginProperties("yes"))
	{
		for (auto& e : in_contract->calls)
		{
			auto& enumData = Wire::ComponentRegistry::EnumData();
			if (enumData.find("eNetEvent") == enumData.end()) continue;
			if (enumData.at("eNetEvent").size() > (uint8_t)e.first)
			{
				UI::Property(enumData.at("eNetEvent")[(uint8_t)e.first], e.second);
			}
		}
		UI::EndProperties();
	}
}

void NetContractPanel::DrawComponentRules(Ref<Volt::NetContract> in_contract)
{
	if (m_entityId == 0 || m_handle == 0) return;
	//auto entity = Volt::Entity(m_entityId, m_scene.get());
	//auto& registry = m_scene->GetRegistry();

	//if (!ImGui::CollapsingHeader("Manage Component Rules")) return;
	//DrawRuleSet(in_contract, m_entityId);
	/*
	for (const auto& [guid, pool] : registry.GetPools())
	{
		if (!registry.HasComponent(guid, m_entityId)) continue;

		const auto& registryInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
		if (registryInfo.name == "MonoScriptComponent")
		{
			auto& monoScriptComponent = registry.GetComponent<Volt::MonoScriptComponent>(m_entityId);
			for (uint32_t i = 0; i < monoScriptComponent.scriptIds.size(); i++)
			{
				RuleEntry(monoScriptComponent.scriptNames[i], in_contract.rules[entity.GetComponent<Volt::PrefabComponent>().prefabEntity][monoScriptComponent.scriptNames[i]]);
			}
			continue;
		}
		RuleEntry(registryInfo.name, in_contract.rules[entity.GetComponent<Volt::PrefabComponent>().prefabEntity][registryInfo.name]);
	}
	*/
}

void NetContractPanel::DrawChildRules(Ref<Volt::NetContract> in_contract)
{
	if (m_entityId == 0 || m_handle == 0) return;

	DrawChild(in_contract, m_entityId);
}

void NetContractPanel::DrawChild(Ref<Volt::NetContract> in_contract, Wire::EntityId in_id)
{
	auto ent = Volt::Entity(in_id, m_scene.get());
	if (ImGui::CollapsingHeader(ent.GetTag().c_str()))
	{
		DrawRuleSet(in_contract, in_id);
	}

	for (auto c : ent.GetComponent<Volt::RelationshipComponent>().Children)
	{
		auto cent = Volt::Entity(c, m_scene.get());
		DrawChild(in_contract, c);
	}
}

void NetContractPanel::RuleEntry(const std::string& in_name, Volt::NetRule& in_rules, const std::string& in_id)
{
	ImGui::Checkbox(("##first" + in_name + in_id).c_str(), &in_rules.host);
	ImGui::SameLine();
	ImGui::Checkbox(("##second" + in_name + in_id).c_str(), &in_rules.owner);
	ImGui::SameLine();
	ImGui::Checkbox(("##third" + in_name + in_id).c_str(), &in_rules.other);
	ImGui::SameLine();
	ImGui::LabelText(("##" + in_name + in_id).c_str(), in_name.c_str());

	auto hoveredItemId = ImGui::GetHoveredID();
	if (hoveredItemId == ImGui::GetCurrentWindow()->GetID(("##first" + in_name + in_id).c_str()))
	{
		ImGui::SetTooltip("Exists on host");
	}
	if (hoveredItemId == ImGui::GetCurrentWindow()->GetID(("##second" + in_name + in_id).c_str()))
	{
		ImGui::SetTooltip("Exists on owner");
	}
	if (hoveredItemId == ImGui::GetCurrentWindow()->GetID(("##third" + in_name + in_id).c_str()))
	{
		ImGui::SetTooltip("Exists on other");
	}
}

bool NetContractPanel::BlockStandardComponents(const std::string& in_name)
{
	if (m_blocked.contains(in_name)) return true;
	return false;
}

void NetContractPanel::DrawRuleSet(Ref<Volt::NetContract> in_contract, Wire::EntityId in_id)
{
	auto& registry = m_scene->GetRegistry();
	auto entity = Volt::Entity(in_id, m_scene.get());
	for (const auto& [guid, pool] : registry.GetPools())
	{
		if (!registry.HasComponent(guid, in_id)) continue;

		const auto& registryInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);

		if (BlockStandardComponents(registryInfo.name)) continue;

		if (registryInfo.name == "MonoScriptComponent")
		{
			auto& monoScriptComponent = registry.GetComponent<Volt::MonoScriptComponent>(in_id);
			for (uint32_t i = 0; i < monoScriptComponent.scriptIds.size(); i++)
			{
				RuleEntry(monoScriptComponent.scriptNames[i], in_contract->rules[entity.GetComponent<Volt::PrefabComponent>().prefabEntity][monoScriptComponent.scriptNames[i]], entity.GetTag());
			}
			continue;
		}
		RuleEntry(registryInfo.name, in_contract->rules[entity.GetComponent<Volt::PrefabComponent>().prefabEntity][registryInfo.name], entity.GetTag());
	}
}



void NetContractPanel::DrawCreateContract()
{
	if (ImGui::Button("Create Contract##NetContractCreate"))
	{
		Volt::NetContractContainer::AddContract(m_handle);
	}
}

