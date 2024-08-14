#include "nexuspch.h"
#include "ReplicationRegistry.h"

namespace Nexus
{
	bool ReplicationRegisty::Unregister(TYPE::REP_ID id)
	{
		if (!IdExist(id))
		{
			LogCritical("REP_ID does not exist");
			return false;
		}
		m_registry.erase(id);
		return true;
	}

	bool ReplicationRegisty::Link(TYPE::REP_ID owner, std::pair<std::string, TYPE::REP_ID> entry)
	{
		if (GetLinked(owner, entry.first) != 0) return false;
		m_links[owner].insert(entry);
		return true;
	}

	bool ReplicationRegisty::Unlink(TYPE::REP_ID owner, std::string key)
	{
		if (GetLinked(owner, key) == 0) return false;
		m_links.at(owner).erase(key);
		return true;
	}

	Ref<Replicated> ReplicationRegisty::Get(TYPE::REP_ID id)
	{
		if (!IdExist(id))
		{
			LogCritical("REP_ID does not exist");
			return nullptr;
		}
		return m_registry[id];
	}

	TYPE::REP_ID ReplicationRegisty::GetLinked(TYPE::REP_ID owner, std::string key)
	{
		if (!m_links.contains(owner)) return 0;
		if (!m_links.at(owner).contains(key)) return 0;
		return m_links.at(owner).at(key);
	}

	Vector<TYPE::REP_ID> ReplicationRegisty::GetAllType(TYPE::eReplicatedType type)
	{
		Vector<TYPE::REP_ID> ret;
		for (auto _pair : m_registry)
		{
			if (_pair.second->GetType() != type) continue;
			ret.push_back(_pair.first);
		}
		return ret;
	}

	Vector<TYPE::REP_ID> ReplicationRegisty::GetAllOwner(TYPE::CLIENT_ID owner)
	{
		Vector<TYPE::REP_ID> ret;
		for (auto _pair : m_registry)
		{
			if (_pair.second->GetOwner() != owner) continue;
			ret.push_back(_pair.first);
		}
		return ret;
	}

	bool ReplicationRegisty::IdExist(TYPE::REP_ID id)
	{
		if (m_registry.find(id) != m_registry.end())
			return true;
		return false;
	}
	TYPE::REP_ID ReplicationRegisty::GetNewId()
	{
		TYPE::REP_ID id = Nexus::RandRepID();
		while (IdExist(id)) id = Nexus::RandRepID();
		return id;
	}
}
