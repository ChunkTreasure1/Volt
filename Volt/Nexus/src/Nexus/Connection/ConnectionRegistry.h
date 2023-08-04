#pragma once
#include "Nexus/API/API.h"
#include "Nexus/Core/Types.h"

namespace Nexus
{
	class ConnectionRegistry
	{
	public:
		[[nodiscard]] TYPE::CLIENT_ID AddConnection(const Nexus::Address& _addr);
		bool RemoveConnection(const TYPE::CLIENT_ID&);
		bool ConnectionExists(const TYPE::CLIENT_ID& _id);
		bool UpdateConnection(const TYPE::CLIENT_ID& _id, const Nexus::Address& _addr);

		Nexus::Address GetSockAddr(const TYPE::CLIENT_ID&);
		bool SockAddrExists(const Nexus::Address& _addr);

		bool UpdateAlias(const TYPE::CLIENT_ID& _id, const std::string& _alias);
		bool AliasExists(const TYPE::CLIENT_ID& _id);
		const std::string GetAlias(const TYPE::CLIENT_ID& _id);

		const std::unordered_map<TYPE::CLIENT_ID, std::string>& GetAliasMap() { return m_aliasMap; }
		const std::unordered_map<TYPE::CLIENT_ID, Nexus::Address>& GetClientIDs() { return m_clientIDs; }

		// Should not be used i think
		bool ValidateConnection(const TYPE::CLIENT_ID& _id, const Nexus::Address& _addr);

		void Clear() { m_clientIDs.clear(); m_aliasMap.clear(); }

	private:
		std::unordered_map<TYPE::CLIENT_ID, Nexus::Address> m_clientIDs;
		std::unordered_map<TYPE::CLIENT_ID, std::string> m_aliasMap;
	};
}
