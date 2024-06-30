#pragma once

#include <CoreUtilities/Containers/Map.h>

#include <typeindex>
#include <any>

namespace Volt
{
	class RenderGraphBlackboard
	{
	public:
		template<typename T>
		inline T& Add()
		{
			static_assert(sizeof(T) < 512 && "Blackboard data is not allowed to be greater than 512 bytes!");

			auto typeIndex = std::type_index{ typeid(T) };

			m_blackboard[typeIndex] = T{};
			return std::any_cast<T&>(m_blackboard.at(typeIndex));
		}

		template<typename T>
		inline T& Get()
		{
			auto typeIndex = std::type_index{ typeid(T) };

			VT_CORE_ASSERT(m_blackboard.contains(typeIndex), "Blackboard does not contain type!");
			return std::any_cast<T&>(m_blackboard.at(typeIndex));
		}

		template<typename T>
		inline const T& Get() const
		{
			auto typeIndex = std::type_index{ typeid(T) };

			VT_CORE_ASSERT(m_blackboard.contains(typeIndex), "Blackboard does not contain type!");
			return std::any_cast<const T&>(m_blackboard.at(typeIndex));
		}

		template<typename T>
		inline const bool Contains() const
		{
			auto typeIndex = std::type_index{ typeid(T) };
			return m_blackboard.contains(typeIndex);
		}

	private:
		uint32_t m_currentDataOffset = 0;
		std::unordered_map<std::type_index, std::any> m_blackboard;
	};
}
