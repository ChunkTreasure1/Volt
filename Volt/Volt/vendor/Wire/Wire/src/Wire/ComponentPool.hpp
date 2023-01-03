#pragma once

#include "Entity.h"

#include "WireGUID.h"

#include <vector>
#include <functional>
#include <unordered_map>
#include <cassert>

namespace Wire
{
	class ComponentPoolBase
	{
	public:
		virtual ~ComponentPoolBase() = default;

		virtual void* AddComponent(EntityId id, void* componentInitData) = 0;
		virtual void* GetComponent(EntityId id) = 0;

		virtual void RemoveComponent(EntityId id) = 0;
		virtual bool HasComponent(EntityId id) = 0;

		virtual void SetOnCreateFunction(std::function<void(EntityId, void*)>& func) = 0;
		virtual void SetOnRemoveFunction(std::function<void(EntityId, void*)>& func) = 0;

		virtual void* GetAllComponents(EntityId id) = 0;
		virtual const std::vector<Wire::EntityId>& GetComponentView() const = 0;

	private:
	};

	template<typename T>
	class ComponentPool : public ComponentPoolBase
	{
	public:
		ComponentPool() = default;
		~ComponentPool() override;

		void* AddComponent(EntityId id, void* componentInitData) override;
		void* GetComponent(EntityId id) override;

		void RemoveComponent(EntityId id) override;
		bool HasComponent(EntityId id) override;

		void SetOnCreateFunction(std::function<void(EntityId, void*)>& func) override;
		void SetOnRemoveFunction(std::function<void(EntityId, void*)>& func) override;

		template<typename... Args>
		T& AddComponent(EntityId id, bool test, Args&&... args);

		void* GetAllComponents(EntityId id) override;
		const std::vector<Wire::EntityId>& GetComponentView() const override;

	private:
		uint32_t m_componentSize = 0;
		WireGUID m_guid = WireGUID::Null();

		std::vector<T> m_pool;

		std::vector<EntityId> m_entitiesWithComponent;
		std::vector<size_t> m_freeSlots;

		std::unordered_map<EntityId, size_t> m_toEntityMap;

		std::function<void(EntityId, void*)> m_onCreate;
		std::function<void(EntityId, void*)> m_onRemove;
	};

	template<typename T>
	inline ComponentPool<T>::~ComponentPool()
	{}

	template<typename T>
	template<typename ...Args>
	inline T& ComponentPool<T>::AddComponent(EntityId id, bool test, Args&& ...args)
	{
		assert(!HasComponent(id));

		size_t slot = m_pool.size();

		if (!m_freeSlots.empty())
		{
			slot = m_freeSlots.back();
			m_freeSlots.pop_back();
		}

		m_toEntityMap[id] = slot;
		m_entitiesWithComponent.emplace_back(id);

		if (slot == m_pool.size())
		{
			T& comp = m_pool.emplace_back(std::forward<Args>(args)...);

			if (m_onCreate)
			{
				m_onCreate(id, &comp);
			}
			return comp;
		}

		T& comp = m_pool.at(slot);
		comp = T(std::forward<Args>(args)...);

		if (m_onCreate)
		{
			m_onCreate(id, &comp);
		}

		return comp;
	}

	template<typename T>
	inline void* ComponentPool<T>::AddComponent(EntityId id, void* componentInitData)
	{
		assert(!HasComponent(id));

		size_t slot = m_pool.size();

		if (!m_freeSlots.empty())
		{
			slot = m_freeSlots.back();
			m_freeSlots.pop_back();
		}

		m_toEntityMap[id] = slot;
		m_entitiesWithComponent.emplace_back(id);

		if (slot == m_pool.size())
		{
			T& comp = m_pool.emplace_back();
			if (componentInitData)
			{
				memcpy_s(&comp, sizeof(T), componentInitData, sizeof(T));
			}

			if (m_onCreate)
			{
				m_onCreate(id, &comp);
			}
			return &comp;
		}

		T& comp = m_pool.at(slot);
		comp = T();

		if (componentInitData)
		{
			memcpy_s(&comp, sizeof(T), componentInitData, sizeof(T));
		}

		if (m_onCreate)
		{
			m_onCreate(id, &comp);
		}

		return &comp;
	}

	template<typename T>
	inline void* ComponentPool<T>::GetComponent(EntityId id)
	{
		assert(HasComponent(id));
		return (void*)&m_pool.at(m_toEntityMap.at(id));
	}

	template<typename T>
	inline void ComponentPool<T>::RemoveComponent(EntityId id)
	{
		assert(HasComponent(id));

		if (m_onRemove)
		{
			void* component = GetComponent(id);
			m_onRemove(id, component);
		}

		auto entIt = std::find(m_entitiesWithComponent.begin(), m_entitiesWithComponent.end(), id);
		m_entitiesWithComponent.erase(entIt);

		auto mapIt = m_toEntityMap.find(id);
		m_freeSlots.emplace_back(mapIt->second);
		m_toEntityMap.erase(mapIt);
	}

	template<typename T>
	inline bool ComponentPool<T>::HasComponent(EntityId id)
	{
		return std::find(m_entitiesWithComponent.begin(), m_entitiesWithComponent.end(), id) != m_entitiesWithComponent.end();
	}

	template<typename T>
	inline void ComponentPool<T>::SetOnCreateFunction(std::function<void(EntityId, void*)>& func)
	{
		m_onCreate = func;
	}

	template<typename T>
	inline void ComponentPool<T>::SetOnRemoveFunction(std::function<void(EntityId, void*)>& func)
	{
		m_onRemove = func;
	}

	template<typename T>
	inline void* ComponentPool<T>::GetAllComponents(EntityId)
	{
		return m_pool.data();
	}

	template<typename T>
	inline const std::vector<Wire::EntityId>& ComponentPool<T>::GetComponentView() const
	{
		return m_entitiesWithComponent;
	}
}