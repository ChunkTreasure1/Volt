#include "nexuspch.h"
#include "ConnectionRegistry.h"
#include "Nexus/Core/Address.h"
#include "Nexus/Utility/Random/Random.h"

namespace Nexus
{
	TYPE::CLIENT_ID ConnectionRegistry::AddConnection(const Nexus::Address& _addr)
	{
		if (SockAddrExists(_addr))
			return 0;

		TYPE::CLIENT_ID clientID = 0;
		while (ConnectionExists(clientID))
			clientID = Random<TYPE::CLIENT_ID>();

		std::pair<TYPE::CLIENT_ID, Nexus::Address> entry{ clientID, _addr };
		m_clientIDs.insert(entry);
		return clientID;
	}

	bool ConnectionRegistry::RemoveConnection(const TYPE::CLIENT_ID& _id)
	{
		if (!ConnectionExists(_id))
			return false;
		auto entry = m_clientIDs.find(_id);
		m_clientIDs.erase(entry);
		// #nexus_todo: remove alias
		return true;
	}

	bool ConnectionRegistry::ConnectionExists(const TYPE::CLIENT_ID& _id)
	{
		if (m_clientIDs.find(_id) != m_clientIDs.end())
			return true;
		return false;
	}

	bool ConnectionRegistry::UpdateConnection(const TYPE::CLIENT_ID& _id, const Nexus::Address& _addr)
	{
		if (ConnectionExists(_id))
			return false;
		m_clientIDs[_id] = _addr;
		return true;
	}

	Nexus::Address ConnectionRegistry::GetSockAddr(const TYPE::CLIENT_ID& _id)
	{
		if (!ConnectionExists(_id))
			return Nexus::Address();
		return m_clientIDs[_id];
	}

	bool ConnectionRegistry::SockAddrExists(const Nexus::Address& _addr)
	{
		for (const auto& entry : m_clientIDs)
		{
			if (entry.second == _addr)
				return true;
		}

		return false;
	}

	bool ConnectionRegistry::UpdateAlias(const TYPE::CLIENT_ID& _id, const std::string& _alias)
	{
		if (!ConnectionExists(_id))
			return false;
		m_aliasMap[_id] = _alias;
		return true;
	}

	bool ConnectionRegistry::AliasExists(const TYPE::CLIENT_ID& _id)
	{
		if (m_aliasMap.find(_id) != m_aliasMap.end())
			return true;
		return false;
	}

	const std::string ConnectionRegistry::GetAlias(const TYPE::CLIENT_ID& _id)
	{
		if (!AliasExists(_id))
			return "NIL";
		return m_aliasMap[_id];
	}

	bool ConnectionRegistry::ValidateConnection(const TYPE::CLIENT_ID& _id, const Nexus::Address& _addr)
	{
		if (ConnectionExists(_id))
			return false;
		return m_clientIDs[_id] == _addr;
	}
}
