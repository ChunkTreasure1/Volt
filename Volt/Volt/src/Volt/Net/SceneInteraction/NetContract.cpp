#include "vtpch.h"
#include "NetContract.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/Net/SceneInteraction/NetActorComponent.h"
#include "Volt/Components/CoreComponents.h"

#include "Volt/Net/SceneInteraction/CallMonoMethod.h"
#include "Volt/Core/Application.h"
#include "Volt/Net/NetHandler.h"
#include "Nexus/Interface/NetManager/NetManager.h"
#include "Volt/Net/Replicated/RepEntity.h"

#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	void NetContractContainer::Execute(Volt::EntityID in_id, eNetEvent in_method, const Vector<uint8_t>& in_data)
	{
		auto entity = SceneManager::GetActiveScene()->GetEntityFromUUID(in_id);
		if (!entity.HasComponent<PrefabComponent>())
		{
			VT_CORE_ERROR("Entity is not prefab in NetContractContainer::Execute");
			return;
		}
		auto handle = entity.GetComponent<PrefabComponent>().prefabAsset;
		CallMonoMethod(entity, GetMethod(handle, in_method), in_data);
	}

	void NetContractContainer::Execute(Nexus::TYPE::REP_ID in_id, eNetEvent in_method, const Vector<uint8_t>& in_data)
	{
		auto* backend = Volt::Application::Get().GetNetHandler().GetBackend().get();
		if (!backend) return;
		auto sceneId = backend->GetRegistry().GetAs<RepEntity>(in_id);
		if (!sceneId) return;
		Execute(sceneId->GetEntityId(), in_method, in_data);
	}

	std::string NetContractContainer::GetMethod(const Ref<NetContract> in_contract, eNetEvent in_event)
	{
		if (!in_contract->calls.contains(in_event)) return "";
		return in_contract->calls.at(in_event);
	}

	std::string NetContractContainer::GetMethod(const AssetHandle& in_handle, eNetEvent in_event)
	{
		if (!m_contracts.contains(in_handle)) return "";
		return GetMethod(m_contracts.at(in_handle), in_event);
	}

	Ref<NetContract> NetContractContainer::GetContract(const AssetHandle& in_handle)
	{
		if (!m_contracts.contains(in_handle)) return m_contracts[AssetHandle(0)];
		return m_contracts.at(in_handle);
	}

	const Ref<NetContract> NetContractContainer::GetContract(Nexus::TYPE::REP_ID in_id)
	{
		auto backend = Volt::Application::Get().GetNetHandler().GetBackend().get();
		if (!backend) return m_contracts[AssetHandle(0)];
		auto sceneId = backend->GetRegistry().GetAs<RepEntity>(in_id);
		if (!sceneId) return m_contracts[AssetHandle(0)];
		return GetContract(sceneId->GetHandle());
	}

	const Ref<NetContract> NetContractContainer::GetContract(Volt::EntityID in_id)
	{
		auto entity = SceneManager::GetActiveScene()->GetEntityFromUUID(in_id);
		if (!entity.HasComponent<PrefabComponent>()) return m_contracts[AssetHandle(0)];
		return GetContract(entity.GetComponent<PrefabComponent>().prefabAsset);
	}

	std::string NetContractContainer::GetMethod(Volt::EntityID in_id, eNetEvent in_event)
	{
		auto entity = SceneManager::GetActiveScene()->GetEntityFromUUID(in_id);
		if (!entity.HasComponent<PrefabComponent>()) return "";
		return GetMethod(entity.GetComponent<PrefabComponent>().prefabAsset, in_event);
	}

	std::string NetContractContainer::GetMethod(Nexus::TYPE::REP_ID in_id, eNetEvent in_event)
	{
		auto entity = SceneManager::GetActiveScene()->GetEntityFromUUID(static_cast<uint32_t>(in_id));
		if (!entity.HasComponent<PrefabComponent>()) return "";
		return GetMethod(entity.GetComponent<PrefabComponent>().prefabAsset, in_event);
	}

	void NetContractContainer::AddContract(const AssetHandle& in_handle)
	{
		if (m_contracts.contains(in_handle)) return;

		const std::filesystem::path contractDirectory = "Assets/Networking/Contracts";
		Ref<NetContract> contract = AssetManager::CreateAsset<NetContract>(contractDirectory, std::to_string(in_handle));
		contract->prefab = in_handle;
		m_contracts.insert({ in_handle, contract });
	}

	bool NetContractContainer::ContractExists(const AssetHandle& in_handle)
	{
		return m_contracts.contains(in_handle);
	}
	bool NetContractContainer::RuleExists(const AssetHandle& in_handle, const std::string& in_rule, Volt::EntityID in_ent)
	{
		if (!m_contracts.contains(in_handle)) return false;
		if (!m_contracts.at(in_handle)->rules.contains(in_ent)) return false;
		return m_contracts.at(in_handle)->rules.at(in_ent).contains(in_rule);
	}

	void NetContractContainer::Load()
	{
		auto pajsdfbklk = ProjectManager::GetDirectory() / "Assets/Networking/Contracts";
		if (!std::filesystem::exists(pajsdfbklk))
		{
			return;
		}
		for (const auto& path : std::filesystem::directory_iterator(pajsdfbklk))
		{
			const auto relPath = AssetManager::GetRelativePath(path.path());
			const auto type = AssetManager::GetAssetTypeFromPath(relPath);

			if (type == AssetType::NetContract)
			{
				if (!AssetManager::ExistsInRegistry(relPath))
				{
					AssetManager::Get().AddAssetToRegistry(relPath);
				}

				Ref<NetContract> contract = AssetManager::GetAsset<NetContract>(relPath);
				m_contracts.insert({ contract->prefab, contract });
			}
		}
	}
	void NetContractContainer::Clear()
	{
		m_contracts.clear();
	}
}
