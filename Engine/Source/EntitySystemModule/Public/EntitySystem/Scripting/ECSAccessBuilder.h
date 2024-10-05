#pragma once

#include "EntitySystem/Scripting/CoreComponents.h"
#include "EntitySystem/EntityID.h"
#include "EntitySystem/EntityHelper.h"

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

		ConstructComponents(const Volt::EntityHelper& entityHelper)
			: m_entityHelper(entityHelper)
		{
			VT_ENSURE(m_entityHelper);
		}

		template<typename Comp>
		Comp& GetComponent()
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Comp>>, ComponentTupleRaw>;

			if constexpr (ComponentTraits::IsValid)
			{
				return m_entityHelper.GetComponent<Comp>();
			}
			else
			{
				using WriteIfExistsTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Comp>>, ComponentTupleWriteIfExists>;

				static_assert(WriteIfExistsTraits::IsValid);
				return m_entityHelper.GetComponent<Comp>();
			}
		}

		template<typename Comp>
		const Comp& GetComponent() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Comp>>, ComponentTupleRaw>;

			if constexpr (ComponentTraits::IsValid)
			{
				return m_entityHelper.GetComponent<Comp>();
			}
			else
			{
				using ReadIfExistsTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Comp>>, ComponentTupleReadIfExists>;
				using WriteIfExistsTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Comp>>, ComponentTupleWriteIfExists>;

				static_assert(ReadIfExistsTraits::IsValid && WriteIfExistsTraits::IsValid);
				return m_entityHelper.GetComponent<Comp>();
			}
		}

		template<typename Comp, typename... Args>
		Comp& AddComponent(Args&&... args)
		{
			m_entityHelper.AddComponent<Comp>(std::forward<Args>(args)...);
		}

		template<typename Comp>
		bool HasComponent()
		{
			return m_entityHelper.HasComponent<Comp>();
		}

		template<typename Comp>
		void RemoveComponent()
		{
			return m_entityHelper.RemoveComponent<Comp>();
		}

		void SetPosition(const glm::vec3& position)
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			m_entityHelper.SetPosition(position);
		}

		void SetRotation(const glm::quat& rotation)
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			m_entityHelper.SetRotation(rotation);
		}

		void SetScale(const glm::vec3& scale)
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			m_entityHelper.SetScale(scale);
		}

		void SetLocalPosition(const glm::vec3& position)
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			m_entityHelper.SetLocalPosition(position);
		}

		void SetLocalRotation(const glm::quat& rotation)
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			m_entityHelper.SetLocalRotation(rotation);
		}

		void SetLocalScale(const glm::vec3& scale)
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			m_entityHelper.SetLocalScale(scale);
		}

		VT_NODISCARD glm::vec3 GetPosition() const 
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetPosition();
		}

		VT_NODISCARD glm::quat GetRotation() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetRotation();
		}

		VT_NODISCARD glm::vec3 GetScale() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetScale();
		}

		VT_NODISCARD glm::vec3 GetLocalPosition() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetLocalPosition();
		}

		VT_NODISCARD glm::quat GetLocalRotation() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetLocalRotation();
		}

		VT_NODISCARD glm::vec3 GetLocalScale() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetLocalScale();
		}

		VT_NODISCARD glm::vec3 GetForward() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetForward();
		}

		VT_NODISCARD glm::vec3 GetRight() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetRight();
		}

		VT_NODISCARD glm::vec3 GetUp() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetUp();
		}

		VT_NODISCARD glm::vec3 GetLocalForward() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetLocalForward();
		}

		VT_NODISCARD glm::vec3 GetLocalRight() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetLocalRight();
		}

		VT_NODISCARD glm::vec3 GetLocalUp() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::TransformComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);
			return m_entityHelper.GetLocalUp();
		}

		VT_NODISCARD Volt::EntityID GetID() const
		{
			using ComponentTraits = Utility::TypeIndex<std::remove_const_t<std::remove_reference_t<Volt::IDComponent>>, ComponentTupleRaw>;
			static_assert(ComponentTraits::IsValid);

			return m_entityHelper.GetID();
		}

		VT_NODISCARD VT_INLINE entt::entity GetHandle() const { return m_entityHelper.GetHandle(); }
		VT_NODISCARD Volt::RenderScene* GetRenderScene() const
		{
			return m_entityHelper.GetSceneReference()->GetRenderScene();
		}

	private:
		Volt::EntityHelper m_entityHelper;
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
				return ConstructComponents<Type::Entity, T...>(*m_iterator, m_registry);
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
			ViewType::iterator m_iterator;
			ViewType& m_view;
			entt::registry& m_registry;
		};

		VT_INLINE constexpr Iterator begin() { return Iterator(m_view.begin(), m_view, m_registry); }
		VT_INLINE constexpr Iterator end() { return Iterator(m_view.end(), m_view, m_registry); }

		VT_INLINE constexpr const Iterator begin() const { return Iterator(m_view.begin(), m_view, m_registry); }
		VT_INLINE constexpr const Iterator end() const { return Iterator(m_view.end(), m_view, m_registry); }

	private:
		mutable ViewType m_view;
		entt::registry& m_registry;
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
