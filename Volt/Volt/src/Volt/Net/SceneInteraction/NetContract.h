#pragma once
#include "Volt/Asset/Asset.h"

#include <Nexus/Core/Packet/PacketID.h>
#include <Nexus/Utility/Types.h>

#include "Volt/Net/Event/NetEvent.h"

#include <entt.hpp>

namespace Volt
{
	struct NetRule
	{
		bool owner = true;
		bool other = true;
		bool host = true;
		bool ShouldKeep(bool in_host, bool in_owner)
		{
			if (in_host && in_owner)
			{
				return host || owner;
			}
			if (!in_host && !in_owner)
			{
				return other;
			}
			if (in_host && !in_owner)
			{
				return host || other;
			}
			if (!in_host && in_owner)
			{
				return owner;
			}
			VT_CORE_ASSERT(false, "OONGA BOONGA something went horribly wrong");
		
			return false;
		}
	};

	class NetContract : public Asset
	{
	public:
		AssetHandle prefab = AssetHandle(0);

		std::unordered_map<eNetEvent, std::string> calls;
		std::unordered_map<entt::entity, std::unordered_map<std::string, NetRule>> rules;

		static AssetType GetStaticType() { return AssetType::NetContract; }
		AssetType GetType() override { return GetStaticType(); }
	};

	class NetContractContainer
	{
	public:
		static void Execute(entt::entity in_id, eNetEvent in_method, const std::vector<uint8_t>& in_data);
		static void Execute(Nexus::TYPE::REP_ID in_id, eNetEvent in_method, const std::vector<uint8_t>& in_data);

		static std::string GetMethod(const AssetHandle& in_handle, eNetEvent in_method);
		static std::string GetMethod(const Ref<NetContract> in_contract, eNetEvent in_method);

		static Ref<NetContract> GetContract(const AssetHandle& in_handle);
		static const Ref<NetContract> GetContract(Nexus::TYPE::REP_ID in_id);
		static const Ref<NetContract> GetContract(entt::entity in_id);

		static std::string GetMethod(entt::entity in_id, eNetEvent in_method);
		static std::string GetMethod(Nexus::TYPE::REP_ID in_id, eNetEvent in_method);

		static void AddContract(const AssetHandle& in_handle);
		static bool ContractExists(const AssetHandle& in_handle);
		static bool RuleExists(const AssetHandle& in_handle, const std::string& in_rule, entt::entity in_ent = entt::null);

		static void Load();
		static void Clear();

	private:
		inline static std::unordered_map<AssetHandle, Ref<NetContract>> m_contracts;
	};
}
