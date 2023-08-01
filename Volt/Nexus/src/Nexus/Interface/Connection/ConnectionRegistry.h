#pragma once
#include "Nexus/API/API.h"
#include "Nexus/Core/Types/Types.h"

namespace Nexus
{
	class ConnectionRegistry
	{
	public:
		[[nodiscard]] TYPE::CLIENT_ID AddConnection(const Nexus::Address& in_addr);
		bool RemoveConnection(const TYPE::CLIENT_ID&);
		bool ConnectionExists(const TYPE::CLIENT_ID& in_id);
		bool UpdateConnection(const TYPE::CLIENT_ID& in_id, const Nexus::Address& in_addr);

		Nexus::Address GetSockAddr(const TYPE::CLIENT_ID&);
		bool SockAddrExists(const Nexus::Address& in_addr);

		bool UpdateAlias(const TYPE::CLIENT_ID& in_id, const std::string& in_alias);
		bool AliasExists(const TYPE::CLIENT_ID& in_id);
		const std::string GetAlias(const TYPE::CLIENT_ID& in_id);

		const std::unordered_map<TYPE::CLIENT_ID, std::string>& GetAliasMap() { return m_aliasMap; }
		const std::unordered_map<TYPE::CLIENT_ID, Nexus::Address>& GetClientIDs() { return m_clientIDs; }

		// Should not be used i think
		bool ValidateConnection(const TYPE::CLIENT_ID& in_id, const Nexus::Address& in_addr);

		void Clear() { m_clientIDs.clear(); m_aliasMap.clear(); }

	private:
		std::unordered_map<TYPE::CLIENT_ID, Nexus::Address> m_clientIDs;
		std::unordered_map<TYPE::CLIENT_ID, std::string> m_aliasMap;
	};
}
