#include "nexuspch.h"
#include "ReplicationRegistry.h"
#include "Nexus/Utility/Random/Random.h"

namespace Nexus
{
	bool ReplicationRegisty::Unregister(TYPE::REP_ID _id)
	{
		if (!IdExist(_id))
		{
			LogCritical("REP_ID does not exist");
			return false;
		}
		m_registry.erase(_id);
		return true;
	}

	bool ReplicationRegisty::Link(TYPE::REP_ID _owner, std::pair<std::string, TYPE::REP_ID> _entry)
	{
		if (GetLinked(_owner, _entry.first) != 0) return false;
		m_links[_owner].insert(_entry);
		return true;
	}

	bool ReplicationRegisty::Unlink(TYPE::REP_ID _owner, std::string _key)
	{
		if (GetLinked(_owner, _key) == 0) return false;
		m_links.at(_owner).erase(_key);
		return true;
	}

	Ref<Replicated> ReplicationRegisty::Get(TYPE::REP_ID _id)
	{
		if (!IdExist(_id))
		{
			LogCritical("REP_ID does not exist");
			return nullptr;
		}
		return m_registry[_id];
	}

	TYPE::REP_ID ReplicationRegisty::GetLinked(TYPE::REP_ID _owner, std::string _key)
	{
		if (!m_links.contains(_owner)) return 0;
		if (!m_links.at(_owner).contains(_key)) return 0;
		return m_links.at(_owner).at(_key);
	}

	std::vector<TYPE::REP_ID> ReplicationRegisty::GetAllType(TYPE::eReplicatedType _type)
	{
		std::vector<TYPE::REP_ID> ret;
		for (auto _pair : m_registry)
		{
			if (_pair.second->GetType() != _type) continue;
			ret.push_back(_pair.first);
		}
		return ret;
	}

	std::vector<TYPE::REP_ID> ReplicationRegisty::GetAllOwner(TYPE::CLIENT_ID _owner)
	{
		std::vector<TYPE::REP_ID> ret;
		for (auto _pair : m_registry)
		{
			if (_pair.second->GetOwner() != _owner) continue;
			ret.push_back(_pair.first);
		}
		return ret;
	}

	bool ReplicationRegisty::IdExist(TYPE::REP_ID _id)
	{
		if (m_registry.find(_id) != m_registry.end())
			return true;
		return false;
	}

	TYPE::REP_ID ReplicationRegisty::GetNewId()
	{
		TYPE::REP_ID id = Random<TYPE::REP_ID>();
		while (IdExist(id)) id = Random<TYPE::REP_ID>();
		return id;
	}
}
