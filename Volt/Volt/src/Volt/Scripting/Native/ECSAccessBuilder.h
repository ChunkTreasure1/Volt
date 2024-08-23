#pragma once

#include <entt.hpp>

namespace ECS
{
	namespace Utility
	{
		template<typename T, typename Tuple>
		struct TypeIndex;

		template<typename T, typename... Types>
		struct TypeIndex<T, std::tuple<T, Types...>>
		{
			static constexpr std::size_t value = 0;
		};

		template<typename T, typename U, typename... Types>
		struct TypeIndex<T, std::tuple<U, Types...>>
		{
			static constexpr std::size_t value = 1 + TypeIndex<T, std::tuple<Types...>>::value;
		};

		template<typename T>
		struct TypeIndex<T, std::tuple<>>
		{
			static_assert(!std::is_same_v<T, T>, "Type not found in tuple!");
		};
	}

	enum class AccessType
	{
		Read,
		Write
	};

	enum class Type
	{
		Entity,
		Query
	};

	template<typename T, AccessType ACCESS_TYPE>
	struct SingleComponentAccess
	{
		typedef T ComponentType;
		inline static constexpr AccessType accessType = ACCESS_TYPE;
	};

	template<typename CompType, bool isWrite>
	struct ComponentType
	{
		using Type = std::conditional_t<isWrite, CompType&, const CompType&>;
	};

	template<Type type, typename... T>
	class ConstructComponents
	{};

	template<typename... T>
	class ConstructComponents<Type::Entity, T...>
	{
	public:
		using ComponentTuple = std::tuple<typename ComponentType<typename T::ComponentType, T::accessType == AccessType::Write>::Type...>;
		using ComponentTupleRaw = std::tuple<std::remove_const_t<std::remove_reference_t<typename ComponentType<typename T::ComponentType, T::accessType == AccessType::Write>::Type>>...>;

		inline static constexpr Type ConstructType = ::Volt::ECS::Type::Entity;

		ConstructComponents(ComponentTuple tuple)
			: m_tuple(tuple)
		{
		}

		ConstructComponents() = default;

		template<typename T>
		T& GetComponent()
		{
			constexpr std::size_t index = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<T>>, ComponentTupleRaw>::value;
			return std::get<index>(m_tuple);
		}

		template<typename T>
		const T& GetComponent() const
		{
			constexpr std::size_t index = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<T>>, ComponentTupleRaw>::value;
			return std::get<index>(m_tuple);
		}

	private:
		ComponentTuple m_tuple;
	};

	template<typename... T>
	class ConstructComponents<Type::Query, T...>
	{
	public:
		using ComponentTuple = std::tuple<typename ComponentType<typename T::ComponentType, T::accessType == AccessType::Write>::Type...>;

		template<typename Type>
		using StorageForType = typename entt::storage_for<std::remove_reference_t<Type>>::type;

		template<typename... Exclude>
		using ViewTypeBase = entt::basic_view<entt::get_t<StorageForType<typename ComponentType<typename T::ComponentType, T::accessType == AccessType::Write>::Type>...>, entt::exclude_t<StorageForType<Exclude>...>>;

		using ViewType = ViewTypeBase<>;

		inline static constexpr Type ConstructType = ::Volt::ECS::Type::Query;

		ConstructComponents(ViewType view)
			: m_view(view)
		{
		}

		constexpr ConstructComponents() = default;

		class Iterator
		{
		public:
			constexpr Iterator(ViewType::iterator it, ViewType& view)
				: m_iterator(it), m_view(view)
			{
			}

			VT_INLINE constexpr ConstructComponents<Type::Entity, T...> operator*() const
			{
				return ConstructComponents<Type::Entity, T...>(GetComponentView<ComponentTuple>(m_view, *m_iterator));
			}

			VT_INLINE constexpr  Iterator& operator++()
			{
				++m_iterator;
				return *this;
			}

			VT_NODISCARD VT_INLINE constexpr const bool operator!=(const Iterator& other) const
			{
				return m_iterator != other.m_iterator;
			}

			VT_NODISCARD VT_INLINE constexpr const bool operator==(const Iterator& other) const
			{
				return m_iterator == other.m_iterator;
			}

		private:
			template<typename Tuple, typename EntityView, std::size_t... Indices>
			static auto GetComponentViewImpl(std::index_sequence<Indices...>, EntityView& view, entt::entity entity)
			{
				return view.get<std::remove_reference_t<std::tuple_element_t<Indices, Tuple>>...>(entity);
			}

			template<typename Tuple, typename EntityView>
			static auto GetComponentView(EntityView& view, entt::entity entity)
			{
				constexpr std::size_t tupleSize = std::tuple_size_v<Tuple>;
				return GetComponentViewImpl<Tuple>(std::make_index_sequence<tupleSize>{}, view, entity);
			}

			ViewType::iterator m_iterator;
			ViewType& m_view;
		};

		VT_INLINE constexpr Iterator begin() { return Iterator(m_view.begin(), m_view); }
		VT_INLINE constexpr Iterator end() { return Iterator(m_view.end(), m_view); }

		VT_INLINE constexpr const Iterator begin() const { return Iterator(m_view.begin(), m_view); }
		VT_INLINE constexpr const Iterator end() const { return Iterator(m_view.end(), m_view); }

	private:
		mutable ViewType m_view;
	};

	template<typename... T>
	struct ComponentAccess
	{
		template<typename ComponentType>
		using Read = ComponentAccess<SingleComponentAccess<ComponentType, AccessType::Read>, T...>;

		template<typename ComponentType>
		using Write = ComponentAccess<SingleComponentAccess<ComponentType, AccessType::Write>, T...>;

		template<Type type>
		using As = ConstructComponents<type, T...>;
	};

	class Access
	{
	public:

		template<typename T>
		using Read = ComponentAccess<SingleComponentAccess<T, AccessType::Read>>;

		template<typename T>
		using Write = ComponentAccess<SingleComponentAccess<T, AccessType::Write>>;
	};
}
