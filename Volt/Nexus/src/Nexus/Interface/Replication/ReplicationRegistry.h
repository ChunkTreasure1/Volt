#pragma once

#include <unordered_map>
#include "Replicated.h"
#include "Nexus/Utility/Log/Log.h"

namespace Nexus
{
	class ReplicationRegisty
	{
	public:
		template< typename T>
		bool Register(TYPE::REP_ID id, T rep);
		bool Unregister(TYPE::REP_ID id);

		bool Link(TYPE::REP_ID owner, std::pair<std::string, TYPE::REP_ID> entry);
		bool Unlink(TYPE::REP_ID owner, std::string key);

		template< typename T>
		Ref<T> GetAs(TYPE::REP_ID id);
		Ref<Replicated> Get(TYPE::REP_ID id);
		TYPE::REP_ID GetLinked(TYPE::REP_ID owner, std::string key);
		std::vector<TYPE::REP_ID> GetAllOwner(TYPE::CLIENT_ID owner);
		std::vector<TYPE::REP_ID> GetAllType(TYPE::eReplicatedType type);

		bool IdExist(TYPE::REP_ID id);
		TYPE::REP_ID GetNewId();

		void Clear() { ClearRegistry(); ClearLinks(); }

	private:
		void ClearRegistry() { m_registry.clear(); }
		void ClearLinks() { m_registry.clear(); }

		// make 1 map from rep id to hieracy id std::unordered_map<TYPE::REP_ID, std::pair<TYPE::HID, Ref<Replicated>>>
		std::unordered_map<TYPE::REP_ID, Ref<Replicated>> m_registry;
		std::unordered_map<TYPE::REP_ID, std::unordered_map<std::string, TYPE::REP_ID>> m_links;
	};

	template< typename T>
	bool ReplicationRegisty::Register(TYPE::REP_ID id, T rep)
	{
		static_assert(std::derived_from<T, Replicated> && "Bad type");

		if (rep.GetType() == TYPE::eReplicatedType::NIL)
			return false;
		if (IdExist(id))
		{
			LogCritical("REP_ID already exists");
			return false;
		}
		m_registry.insert({ id, CreateRef<T>(rep) });
		return true;
	}

	template<typename T>
	inline Ref<T> ReplicationRegisty::GetAs(TYPE::REP_ID id)
	{
		static_assert(std::derived_from<T, Replicated> && "Bad type");

		Ref<Replicated> rep = Get(id);
		if (!rep) return nullptr;
		return reinterpret_pointer_cast<T>(rep);
	}
}
