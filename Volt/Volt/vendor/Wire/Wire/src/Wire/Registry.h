#pragma once

#include "Entity.h"
#include "WireGUID.h"
#include "ComponentPool.hpp"

#include <memory>
#include <tuple>
#include <execution>

namespace Wire
{
	class Registry
	{
	public:
		Registry() = default;
		Registry(const Registry& registry);
		~Registry();

		EntityId CreateEntity();
		EntityId AddEntity(EntityId aId);

		void RemoveEntity(EntityId aId);
		void Clear();
		void Sort(std::function<bool(const uint32_t lhs, const uint32_t rhs)> sortFunc);
		bool Exists(EntityId aId) const;

		inline const std::vector<EntityId>& GetAllEntities() const { return m_usedIds; }
		inline const std::unordered_map<WireGUID, std::shared_ptr<ComponentPoolBase>>& GetPools() const { return m_pools; }

		bool HasComponent(WireGUID guid, EntityId id) const;
		void RemoveComponent(WireGUID guid, EntityId id);
		void* GetComponentPtr(WireGUID guid, EntityId id) const;
		void* AddComponent(WireGUID guid, EntityId id);

		template<typename T, typename... Args>
		T& AddComponent(EntityId aEntity, Args&&... args);

		template<typename T>
		T& GetComponent(EntityId aEntity);

		template<typename T>
		bool HasComponent(EntityId aEntity) const;

		template<typename ... T>
		bool HasComponents(EntityId aEntity) const;

		template<typename T>
		void RemoveComponent(EntityId aEntity);

		template<typename T>
		const std::vector<T>& GetAllComponents() const;

		template<typename ... T>
		const std::vector<EntityId> GetComponentView() const;
		const std::vector<EntityId> GetComponentView(WireGUID) const;

		template<typename T>
		const std::vector<EntityId>& GetSingleComponentView() const;

		template<typename T>
		void SetOnCreateFunction(std::function<void(EntityId, void*)>&& func);

		template<typename T>
		void SetOnRemoveFunction(std::function<void(EntityId, void*)>&& func);

		template<typename ... T, typename F>
		void ForEach(F&& func);

		template<typename ... T, typename F>
		void ForEachPar(F&& func);

		template<typename ... T, typename F>
		void ForEachSafe(F&& func);

	private:
		std::unordered_map<WireGUID, std::shared_ptr<ComponentPoolBase>> m_pools;

		EntityId m_nextEntityId = 1; // ID zero is null
		std::vector<EntityId> m_usedIds;
	};

	template<typename T, typename... Args>
	inline T& Registry::AddComponent(EntityId aEntity, Args&&... args)
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			auto pool = it->second;
			std::shared_ptr<ComponentPool<T>> poolOfType = std::reinterpret_pointer_cast<ComponentPool<T>>(pool);

			return poolOfType->AddComponent(aEntity, false, std::forward<Args>(args)...);
		}
		else
		{
			m_pools.emplace(guid, CreateRef<ComponentPool<T>>());
			std::shared_ptr<ComponentPool<T>> poolOfType = std::reinterpret_pointer_cast<ComponentPool<T>>(m_pools.at(guid));

			return poolOfType->AddComponent(aEntity, false, std::forward<Args>(args)...);
		}
	}

	template<typename T>
	inline T& Registry::GetComponent(EntityId aEntity)
	{
		assert(HasComponent<T>(aEntity));
		const WireGUID guid = T::comp_guid;

		return *(T*)m_pools.at(guid)->GetComponent(aEntity);
	}

	template<typename T>
	inline bool Registry::HasComponent(EntityId aEntity) const
	{
		const WireGUID guid = T::comp_guid;
		if (m_pools.find(guid) == m_pools.end())
		{
			return false;
		}

		return m_pools.at(guid)->HasComponent(aEntity);
	}

	template<typename ...T>
	inline bool Registry::HasComponents(EntityId aEntity) const
	{
		return (HasComponent<T>(aEntity) && ...);
	}

	template<typename T>
	inline void Registry::RemoveComponent(EntityId aEntity)
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		assert(it != m_pools.end());

		it->second->RemoveComponent(aEntity);
	}

	template<typename T>
	inline const std::vector<T>& Registry::GetAllComponents() const
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		assert(it != m_pools.end());

		return reinterpret_cast<const std::vector<T>&>(it->second.GetAllComponents());
	}

	template<typename ... T>
	inline const std::vector<EntityId> Registry::GetComponentView() const
	{
		std::vector<EntityId> entities;

		for (const auto& ent : m_usedIds)
		{
			if (HasComponents<T...>(ent))
			{
				entities.emplace_back(ent);
			}
		}

		return entities;
	}

	inline const std::vector<EntityId> Registry::GetComponentView(WireGUID guid) const
	{
		std::vector<EntityId> entities;

		for (const auto& ent : m_usedIds)
		{
			if (HasComponent(guid, ent))
			{
				entities.emplace_back(ent);
			}
		}

		return entities;
	}

	template<typename T>
	inline const std::vector<EntityId>& Registry::GetSingleComponentView() const
	{
		const WireGUID guid = T::comp_guid;
		
		if (!m_pools.contains(guid))
		{
			static std::vector<EntityId> emptyVector;
			return emptyVector;
		}

		return m_pools.at(guid)->GetComponentView();
	}

	template<typename T>
	inline void Registry::SetOnCreateFunction(std::function<void(EntityId, void*)>&& func)
	{
		const WireGUID guid = T::comp_guid;

		if (!m_pools.contains(guid))
		{
			m_pools.emplace(guid, CreateRef<ComponentPool<T>>());
		}

		m_pools.at(guid)->SetOnCreateFunction(func);
	}

	template<typename T>
	inline void Registry::SetOnRemoveFunction(std::function<void(EntityId, void*)> && func)
	{
		const WireGUID guid = T::comp_guid;

		if (!m_pools.contains(guid))
		{
			m_pools.emplace(guid, CreateRef<ComponentPool<T>>());
		}

		m_pools.at(guid)->SetOnRemoveFunction(func);
	}

	template<typename ...T, typename F>
	inline void Registry::ForEach(F&& func)
	{
		using FirstType = std::tuple_element<0, std::tuple<T...> >::type;

		const auto& view = GetSingleComponentView<FirstType>();
		for (const auto& ent : view)
		{
			if (HasComponents<T...>(ent))
			{
				func(ent, GetComponent<T>(ent)...);
			}
		}
	}

	template<typename ...T, typename F>
	inline void Registry::ForEachPar(F&& func)
	{
		using FirstType = std::tuple_element<0, std::tuple<T...> >::type;

		const auto& view = GetSingleComponentView<FirstType>();
		std::for_each(std::execution::par, view.begin(), view.end(), [&](Wire::EntityId id) 
			{
				for (const auto& ent : view)
				{
					if (HasComponents<T...>(ent))
					{
						func(ent, GetComponent<T>(ent)...);
					}
				}
			});
	}

	template<typename ...T, typename F>
	inline void Registry::ForEachSafe(F&& func)
	{
		using FirstType = std::tuple_element<0, std::tuple<T...> >::type;

		const auto view = GetSingleComponentView<FirstType>();
		const int64_t size = (int64_t)view.size();

		for (int64_t i = size - 1; i >= 0; --i)
		{
			const auto& id = view.at(i);

			if (HasComponents<T...>(id))
			{
				func(id, GetComponent<T>(id)...);
			}
		}
	}
}
