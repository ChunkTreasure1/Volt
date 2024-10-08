#pragma once

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/TypeTraits/TypeIndex.h>

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
			static_assert(sizeof(T) < 1024 && "Blackboard data is not allowed to be greater than 1024 bytes!");

			auto typeIndex = TypeTraits::TypeIndex::FromType<T>();

			m_blackboard[typeIndex] = T{};
			return std::any_cast<T&>(m_blackboard.at(typeIndex));
		}

		template<typename T>
		inline T& Get()
		{
			auto typeIndex = TypeTraits::TypeIndex::FromType<T>();

			VT_ASSERT_MSG(m_blackboard.contains(typeIndex), "Blackboard does not contain type!");
			return std::any_cast<T&>(m_blackboard.at(typeIndex));
		}

		template<typename T>
		inline const T& Get() const
		{
			auto typeIndex = TypeTraits::TypeIndex::FromType<T>();

			VT_ASSERT_MSG(m_blackboard.contains(typeIndex), "Blackboard does not contain type!");
			return std::any_cast<const T&>(m_blackboard.at(typeIndex));
		}

		template<typename T>
		inline const bool Contains() const
		{
			auto typeIndex = TypeTraits::TypeIndex::FromType<T>();
			return m_blackboard.contains(typeIndex);
		}

	private:
		uint32_t m_currentDataOffset = 0;
		std::unordered_map<TypeTraits::TypeIndex, std::any> m_blackboard;
	};
}
