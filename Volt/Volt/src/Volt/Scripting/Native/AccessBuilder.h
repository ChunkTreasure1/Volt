#pragma once

namespace Volt::ECS
{
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
	{};

	template<typename CompType>
	struct ComponentType<CompType, false>
	{
		using Type = CompType&;
	};

	template<typename CompType>
	struct ComponentType<CompType, true>
	{
		using Type = const CompType&;
	};

	template<Type type, typename... T>
	class ConstructComponents
	{};

	template<typename... T>
	class ConstructComponents<Type::Entity, T...>
	{
	public:
		using ComponentTuple = std::tuple<typename ComponentType<typename T::ComponentType, T::accessType == AccessType::Write>::Type...>;
		ConstructComponents(ComponentTuple tuple)
			: m_tuple(tuple)
		{
		}

		ConstructComponents() = default;

		template<typename T>
		T& GetComponent()
		{
			constexpr std::size_t index = TypeIndex<T, ComponentTuple>::value;
			return std::get<index>(m_tuple);
		}

		template<typename T>
		const T& GetComponent() const
		{
			constexpr std::size_t index = TypeIndex<T, ComponentTuple>::value;
			return std::get<index>(m_tuple);
		}

	private:
		template<typename T, typename Tuple>
		struct TypeIndex;

		template<typename T, typename... Types>
		struct TypeIndex<T, std::tuple<T, Types...>> : std::integral_constant<std::size_t, 0> {};

		template<typename T, typename U, typename... Types>
		struct TypeIndex<T, std::tuple<U, Types...>> : std::integral_constant<std::size_t, 1 + TypeIndex<T, std::tuple<Types...>>::value> {};

		ComponentTuple m_tuple;
	};

	template<typename... T>
	class ConstructComponents<Type::Query, T...>
	{
	public:
		using ComponentTuple = std::tuple<typename ComponentType<typename T::ComponentType, T::accessType == AccessType::Write>::Type...>;
		ConstructComponents(const ComponentTuple& tuple)
		{
		}

		ConstructComponents() = default;

	private:
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
