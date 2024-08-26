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
			static constexpr std::size_t Value = 0;
			static constexpr bool IsValid = true;
		};

		template<typename T, typename U, typename... Types>
		struct TypeIndex<T, std::tuple<U, Types...>>
		{
			static constexpr std::size_t Value = 1 + TypeIndex<T, std::tuple<Types...>>::Value;
			static constexpr bool IsValid = true;
		};

		template<typename T>
		struct TypeIndex<T, std::tuple<>>
		{
			static constexpr bool IsValid = false;
		};
	}

	enum class AccessType
	{
		Read,
		Write,
		With,
		Without,
		ReadIfExists,
		WriteIfExists
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
	struct ComponentTraits
	{
		using Type = std::conditional_t<isWrite, CompType&, const CompType&>;
	};

	template<AccessType... types>
	struct IsNot
	{
		template<typename T>
		struct apply : std::bool_constant<((T::accessType != types) && ...)> {};
	};

	template<AccessType... types>
	struct Is
	{
		template<typename T>
		struct apply : std::bool_constant<((T::accessType == types) && ...)> {};
	};

	using IsWithout = Is<AccessType::Without>;
	using IsNotWithout = IsNot<AccessType::Without>;
	using IsNotWithoutOrWith = IsNot<AccessType::Without, AccessType::With>;

	template<typename Filter, bool RemoveConstRef, typename... Ts>
	struct FilterComponents;

	template<typename Filter, bool RemoveConstRef, typename T, typename... Ts>
	struct FilterComponents<Filter, RemoveConstRef, T, Ts...>
	{
		using Type = std::conditional_t<RemoveConstRef,
			std::conditional_t<
				Filter::template apply<T>::value,
				decltype(std::tuple_cat(
					std::declval<std::tuple<std::remove_const_t<std::remove_reference_t<typename ComponentTraits<typename T::ComponentType, T::accessType == AccessType::Write>::Type>>>>(),
					std::declval<typename FilterComponents<Filter, RemoveConstRef, Ts...>::Type>()
				)),
				typename FilterComponents<Filter, RemoveConstRef, Ts...>::Type
			>,
				std::conditional_t<
				Filter::template apply<T>::value,
				decltype(std::tuple_cat(
					std::declval<std::tuple<typename ComponentTraits<typename T::ComponentType, T::accessType == AccessType::Write>::Type>>(),
					std::declval<typename FilterComponents<Filter, RemoveConstRef, Ts...>::Type>()
				)),
				typename FilterComponents<Filter, RemoveConstRef, Ts...>::Type
			>
		>;
	};

	template<typename Filter, bool RemoveConstRef>
	struct FilterComponents<Filter, RemoveConstRef>
	{
		using Type = std::tuple<>;
	};

	template<Type type, typename... T>
	class ConstructComponents
	{};

	template<typename... T>
	class ConstructComponents<Type::Entity, T...>
	{
	public:
		using ComponentViewTuple = typename FilterComponents<IsNotWithout, false, T...>::Type;
		using ComponentViewExcludeTuple = typename FilterComponents<IsWithout, false, T...>::Type;
		using ComponentTuple = typename FilterComponents<IsNotWithoutOrWith, false, T...>::Type;
		using ComponentTupleRaw = typename FilterComponents<IsNotWithoutOrWith, true, T...>::Type;

		using ComponentTupleReadIfExists = typename FilterComponents<Is<AccessType::ReadIfExists>, true, T...>::Type;
		using ComponentTupleWriteIfExists = typename FilterComponents<Is<AccessType::WriteIfExists>, true, T...>::Type;

		inline static constexpr Type ConstructType = ::ECS::Type::Entity;

		ConstructComponents(ComponentTuple tuple, entt::entity entityId, entt::registry& registry)
			: m_tuple(tuple), m_entityId(entityId), m_registry(registry)
		{
		}

		template<typename T>
		T& GetComponent()
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<T>>, ComponentTupleRaw>;

			if constexpr (ComponentTraits::IsValid)
			{
				constexpr std::size_t index = ComponentTraits::Value;
				return std::get<index>(m_tuple);
			}
			else
			{
				using WriteIfExistsTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<T>>, ComponentTupleWriteIfExists>;

				static_assert(WriteIfExistsTraits::IsValid);
				return m_registry->get<T>(m_entityId);
			}
		}

		template<typename T>
		const T& GetComponent() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<T>>, ComponentTupleRaw>;

			if constexpr (ComponentTraits::IsValid)
			{
				constexpr std::size_t index = ComponentTraits::Value;
				return std::get<index>(m_tuple);
			}
			else
			{
				using ReadIfExistsTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<T>>, ComponentTupleReadIfExists>;
				using WriteIfExistsTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<T>>, ComponentTupleWriteIfExists>;

				static_assert(ReadIfExistsTraits::IsValid && WriteIfExistsTraits::IsValid);
				return m_registry.get<T>(m_entityId);
			}
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			m_registry.emplace<T>(m_entityId, std::forward<Args>(args)...);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_registry.any_of<T>(m_entityId);
		}

		template<typename T>
		void RemoveComponent()
		{
			return m_registry.remove<T>(m_entityId);
		}

	private:
		ComponentTuple m_tuple;
		entt::entity m_entityId;
		entt::registry& m_registry;
	};

	template<typename T>
	struct StorageForType
	{
		using Type = typename entt::storage_for<std::remove_reference_t<T>>::type;
	};

	template<typename Tuple, template<typename...> class TargetTemplate>
	struct ExpandAndApply;

	template<template<typename...> class TargetTemplate, typename... Ts>
	struct ExpandAndApply<std::tuple<Ts...>, TargetTemplate>
	{
		using Type = TargetTemplate<typename StorageForType<Ts>::Type...>;
	};

	template<typename... T>
	class ConstructComponents<Type::Query, T...>
	{
	public:
		using ComponentViewTuple = typename FilterComponents<IsNotWithout, false, T...>::Type;
		using ComponentViewExcludeTuple = typename FilterComponents<IsWithout, false, T...>::Type;
		using ComponentTuple = typename FilterComponents<IsNotWithoutOrWith, false, T...>::Type;

		using ViewType = entt::basic_view<typename ExpandAndApply<ComponentViewTuple, entt::get_t>::Type, typename ExpandAndApply<ComponentViewExcludeTuple, entt::exclude_t>::Type>;

		inline static constexpr Type ConstructType = ::ECS::Type::Query;

		ConstructComponents(ViewType view, entt::registry& registry)
			: m_view(view), m_registry(registry)
		{
		}

		constexpr ConstructComponents() = default;

		class Iterator
		{
		public:
			constexpr Iterator(ViewType::iterator it, ViewType& view, entt::registry& registry)
				: m_iterator(it), m_view(view), m_registry(registry)
			{
			}

			VT_INLINE constexpr ConstructComponents<Type::Entity, T...> operator*() const
			{
				return ConstructComponents<Type::Entity, T...>(GetComponentView<ComponentTuple>(m_view, *m_iterator), *m_iterator, m_registry);
			}

			VT_INLINE constexpr Iterator& operator++()
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
			entt::registry& m_registry = nullptr;
		};

		VT_INLINE constexpr Iterator begin() { return Iterator(m_view.begin(), m_view, m_registry); }
		VT_INLINE constexpr Iterator end() { return Iterator(m_view.end(), m_view, m_registry); }

		VT_INLINE constexpr const Iterator begin() const { return Iterator(m_view.begin(), m_view, m_registry); }
		VT_INLINE constexpr const Iterator end() const { return Iterator(m_view.end(), m_view, m_registry); }

	private:
		mutable ViewType m_view;
		entt::registry& m_registry = nullptr;
	};

	template<typename... T>
	struct ComponentAccess
	{
		template<typename ComponentTraits>
		using Read = ComponentAccess<SingleComponentAccess<ComponentTraits, AccessType::Read>, T...>;

		template<typename ComponentTraits>
		using Write = ComponentAccess<SingleComponentAccess<ComponentTraits, AccessType::Write>, T...>;

		template<typename ComponentTraits>
		using With = ComponentAccess<SingleComponentAccess<ComponentTraits, AccessType::With>, T...>;

		template<typename ComponentTraits>
		using Without = ComponentAccess<SingleComponentAccess<ComponentTraits, AccessType::Without>, T...>;

		template<typename ComponentTraits>
		using ReadIfExists = ComponentAccess<SingleComponentAccess<ComponentTraits, AccessType::ReadIfExists>, T...>;

		template<typename ComponentTraits>
		using WriteIfExists = ComponentAccess<SingleComponentAccess<ComponentTraits, AccessType::WriteIfExists>, T...>;

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

		template<typename T>
		using With = ComponentAccess<SingleComponentAccess<T, AccessType::With>>;

		template<typename T>
		using Without = ComponentAccess<SingleComponentAccess<T, AccessType::Without>>;

		template<typename T>
		using ReadIfExists = ComponentAccess<SingleComponentAccess<T, AccessType::ReadIfExists>>;

		template<typename T>
		using WriteIfExists = ComponentAccess<SingleComponentAccess<T, AccessType::WriteIfExists>>;
	};
}
