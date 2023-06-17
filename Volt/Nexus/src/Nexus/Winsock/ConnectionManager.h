#pragma once
#include "Nexus/Core/Core.h"

namespace Nexus
{
	class ConnectionManager
	{
	public:
		[[nodiscard]] TYPE::CLIENT_ID AddConnection(const sockaddr_in& in_addr);
		bool RemoveConnection(const TYPE::CLIENT_ID&);
		bool ConnectionExists(const TYPE::CLIENT_ID& in_id);
		bool UpdateConnection(const TYPE::CLIENT_ID& in_id, const sockaddr_in& in_addr);

		sockaddr_in GetSockAddr(const TYPE::CLIENT_ID&);
		bool SockAddrExists(const sockaddr_in& in_addr);

		bool UpdateAlias(const TYPE::CLIENT_ID& in_id, const std::string& in_alias);
		bool AliasExists(const TYPE::CLIENT_ID& in_id);
		const std::string GetAlias(const TYPE::CLIENT_ID& in_id);

		const std::unordered_map<TYPE::CLIENT_ID, std::string>& GetAliasMap() { return m_aliasMap; }
		const std::unordered_map<TYPE::CLIENT_ID, sockaddr_in>& GetClientIDs() { return m_clientIDs; }
		std::vector<TYPE::CLIENT_ID> GetIdList();

		// Should not be used i think
		bool ValidateConnection(const TYPE::CLIENT_ID& in_id, const sockaddr_in& in_addr);

		void Clear() { m_clientIDs.clear(); m_aliasMap.clear(); }

	private:
		std::unordered_map<TYPE::CLIENT_ID, sockaddr_in> m_clientIDs;
		std::unordered_map<TYPE::CLIENT_ID, std::string> m_aliasMap;
	};
}
