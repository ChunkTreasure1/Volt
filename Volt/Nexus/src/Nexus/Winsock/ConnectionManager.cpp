#include "nexuspch.h"
#include "ConnectionManager.h"

namespace Nexus
{
	TYPE::CLIENT_ID ConnectionManager::AddConnection(const sockaddr_in& in_addr)
	{
		if (SockAddrExists(in_addr))
			return 0;

		TYPE::CLIENT_ID clientID = 0;
		while (ConnectionExists(clientID))
			clientID = RandClientID();

		std::pair<TYPE::CLIENT_ID, sockaddr_in> entry{ clientID, in_addr };
		m_clientIDs.insert(entry);
		return clientID;
	}

	bool ConnectionManager::RemoveConnection(const TYPE::CLIENT_ID& in_id)
	{
		if (!ConnectionExists(in_id))
			return false;
		auto entry = m_clientIDs.find(in_id);
		m_clientIDs.erase(entry);
		// #nexus_todo: remove alias
		return true;
	}

	bool ConnectionManager::ConnectionExists(const TYPE::CLIENT_ID& in_id)
	{
		if (m_clientIDs.find(in_id) != m_clientIDs.end())
			return true;
		return false;
	}

	bool ConnectionManager::UpdateConnection(const TYPE::CLIENT_ID& in_id, const sockaddr_in& in_addr)
	{
		if (ConnectionExists(in_id))
			return false;
		m_clientIDs[in_id] = in_addr;
		return true;
	}

	sockaddr_in ConnectionManager::GetSockAddr(const TYPE::CLIENT_ID& in_id)
	{
		if (!ConnectionExists(in_id))
			return sockaddr_in();
		return m_clientIDs[in_id];
	}

	bool ConnectionManager::SockAddrExists(const sockaddr_in& in_addr)
	{
		for (const auto& entry : m_clientIDs)
		{
			if (entry.second == in_addr)
				return true;
		}

		return false;
	}

	bool ConnectionManager::UpdateAlias(const TYPE::CLIENT_ID& in_id, const std::string& in_alias)
	{
		if (!ConnectionExists(in_id))
			return false;
		m_aliasMap[in_id] = in_alias;
		return true;
	}

	bool ConnectionManager::AliasExists(const TYPE::CLIENT_ID& in_id)
	{
		if (m_aliasMap.find(in_id) != m_aliasMap.end())
			return true;
		return false;
	}

	const std::string ConnectionManager::GetAlias(const TYPE::CLIENT_ID& in_id)
	{
		if (!AliasExists(in_id))
			return "NIL";
		return m_aliasMap[in_id];
	}

	Vector<TYPE::CLIENT_ID> ConnectionManager::GetIdList()
	{
		Vector<TYPE::CLIENT_ID> ret;
		for (auto _pair : m_clientIDs)
		{
			ret.push_back(_pair.first);
		}
		return ret;
	}

	bool ConnectionManager::ValidateConnection(const TYPE::CLIENT_ID& in_id, const sockaddr_in& in_addr)
	{
		if (ConnectionExists(in_id))
			return false;
		return m_clientIDs[in_id] == in_addr;
	}
}
