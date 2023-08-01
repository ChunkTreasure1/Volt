#include "nexuspch.h"
#include "ConnectionRegistry.h"
#include "Nexus/Core/Address.h"

namespace Nexus
{
	TYPE::CLIENT_ID ConnectionRegistry::AddConnection(const Nexus::Address& in_addr)
	{
		if (SockAddrExists(in_addr))
			return 0;

		TYPE::CLIENT_ID clientID = 0;
		while (ConnectionExists(clientID))
			clientID = TYPE::RandClientID();

		std::pair<TYPE::CLIENT_ID, Nexus::Address> entry{ clientID, in_addr };
		m_clientIDs.insert(entry);
		return clientID;
	}

	bool ConnectionRegistry::RemoveConnection(const TYPE::CLIENT_ID& in_id)
	{
		if (!ConnectionExists(in_id))
			return false;
		auto entry = m_clientIDs.find(in_id);
		m_clientIDs.erase(entry);
		// #nexus_todo: remove alias
		return true;
	}

	bool ConnectionRegistry::ConnectionExists(const TYPE::CLIENT_ID& in_id)
	{
		if (m_clientIDs.find(in_id) != m_clientIDs.end())
			return true;
		return false;
	}

	bool ConnectionRegistry::UpdateConnection(const TYPE::CLIENT_ID& in_id, const Nexus::Address& in_addr)
	{
		if (ConnectionExists(in_id))
			return false;
		m_clientIDs[in_id] = in_addr;
		return true;
	}

	Nexus::Address ConnectionRegistry::GetSockAddr(const TYPE::CLIENT_ID& in_id)
	{
		if (!ConnectionExists(in_id))
			return Nexus::Address();
		return m_clientIDs[in_id];
	}

	bool ConnectionRegistry::SockAddrExists(const Nexus::Address& in_addr)
	{
		for (const auto& entry : m_clientIDs)
		{
			if (entry.second == in_addr)
				return true;
		}

		return false;
	}

	bool ConnectionRegistry::UpdateAlias(const TYPE::CLIENT_ID& in_id, const std::string& in_alias)
	{
		if (!ConnectionExists(in_id))
			return false;
		m_aliasMap[in_id] = in_alias;
		return true;
	}

	bool ConnectionRegistry::AliasExists(const TYPE::CLIENT_ID& in_id)
	{
		if (m_aliasMap.find(in_id) != m_aliasMap.end())
			return true;
		return false;
	}

	const std::string ConnectionRegistry::GetAlias(const TYPE::CLIENT_ID& in_id)
	{
		if (!AliasExists(in_id))
			return "NIL";
		return m_aliasMap[in_id];
	}

	bool ConnectionRegistry::ValidateConnection(const TYPE::CLIENT_ID& in_id, const Nexus::Address& in_addr)
	{
		if (ConnectionExists(in_id))
			return false;
		return m_clientIDs[in_id] == in_addr;
	}
}
