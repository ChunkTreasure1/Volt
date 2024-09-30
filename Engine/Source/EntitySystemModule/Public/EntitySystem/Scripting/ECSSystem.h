#pragma once

#include "EntitySystem/Config.h"

#include "ECSAccessBuilder.h"

namespace Volt
{
	class EntityScene;
}

class ECSSystem;

class VTES_API ECSExecutionOrder
{
public:
	void ExecuteAfter(ECSSystem& otherSystem);
	void ExecuteBefore(ECSSystem& otherSystem);

private:
	friend class ECSGameLoopContainer;

	Vector<UUID64> m_executeBefore;
	Vector<UUID64> m_executeAfter;
};

using ECSSystemFunc = std::function<void(Volt::EntityScene& registry, float deltaTime)>;

struct ComponentAccess
{
	VoltGUID componentGUID;
	bool write = false;
};

class VTES_API ECSSystem
{
public:
	ECSSystem() = default;
	ECSSystem(UUID64 id, ECSSystemFunc&& func);
	~ECSSystem();

	void Execute(Volt::EntityScene& scene, float deltaTime);

	VT_NODISCARD VT_INLINE ECSExecutionOrder& Order() { return m_executionOrder; }
	VT_NODISCARD VT_INLINE UUID64 GetID() const { return m_id; }

private:
	template<typename Ret, typename... Args>
	friend class ECSSystemRegisterer;

	ECSSystemFunc m_systemFunc;
	ECSExecutionOrder m_executionOrder;

	Vector<ComponentAccess> m_componentAccesses;
	UUID64 m_id = 0;
};

template<typename Ret, typename... Args>
class ECSSystemRegisterer
{
public:
	template<typename T>
	struct FunctionTraits;

	template<typename Ret, typename... Args>
	struct FunctionTraits<Ret(*)(Args...)>
	{
		using ReturnType = Ret;
		using ArgumentTypes = std::tuple<Args...>;
	};

	using Traits = FunctionTraits<Ret(*)(Args...)>;
	using ArgumentTypes = typename Traits::ArgumentTypes;
	using ComponentView = std::tuple_element_t<0, ArgumentTypes>;
	using ComponentTuple = typename ComponentView::ComponentViewTuple;

	ECSSystem operator()(Ret(*func)(Args...))
	{
		// A system must have at least one argument...
		// And the first should be an entity type
		static_assert(std::tuple_size_v<ArgumentTypes> > 0);
		//static_assert(std::is_same_v<std::tuple_element_t<std::tuple_size_v<ArgumentTypes> -1, ArgumentTypes>, float>);
		static_assert(ComponentView::ConstructType == ECS::Type::Entity);
	
		constexpr bool IsFinalArgFloat = std::is_same_v<std::tuple_element_t<std::tuple_size_v<ArgumentTypes> -1, ArgumentTypes>, float>;

		UUID64 id{};

		auto systemFunc = [func](Volt::EntityScene& scene, float deltaTime = 0.f)
		{
			auto view = GetRegistryView<ComponentTuple>(scene.GetRegistry());
			auto deltaTimeTuple = std::tuple{ deltaTime };

			for (const auto& entity : view)
			{
				auto arguments = GetSystemArgument<ArgumentTypes, IsFinalArgFloat>(scene, view, entity);

				if constexpr (IsFinalArgFloat)
				{
					auto finalArguments = std::tuple_cat(arguments, deltaTimeTuple);
					std::apply(func, finalArguments);
				}
				else
				{
					std::apply(func, arguments);
				}
			}
		};

		ECSSystem result;
		result.m_id = id;
		result.m_systemFunc = std::move(systemFunc);
		result.m_componentAccesses = GetComponentAccesses<ArgumentTypes, IsFinalArgFloat>();

		return result;
	}

	Vector<ComponentAccess> GetSystemComponentAccesses()
	{
		constexpr bool IsFinalArgFloat = std::is_same_v<std::tuple_element_t<std::tuple_size_v<ArgumentTypes> -1, ArgumentTypes>, float>;
		return GetComponentAccesses<ArgumentTypes, IsFinalArgFloat>();
	}

private:
	// Will iterate each access type
	template<typename T>
	static auto GetComponentAccess()
	{
		const auto guid = Volt::GetTypeGUID<std::remove_const_t<std::remove_reference_t<T>>>();
		
		return ComponentAccess{ guid, !std::is_const_v<std::remove_reference_t<T>> };
	}

	template<typename Tuple, std::size_t... Indices>
	static auto GetComponentTupleAccessesImpl(std::index_sequence<Indices...>)
	{
		return Vector<ComponentAccess>{ GetComponentAccess<std::tuple_element_t<Indices, Tuple>>()... };
	}

	template<typename ComponentTuple>
	static auto GetComponentTupleAccesses()
	{
		constexpr std::size_t tupleSize = std::tuple_size_v<ComponentTuple>;
		return GetComponentTupleAccessesImpl<ComponentTuple>(std::make_index_sequence<tupleSize>{});
	}

	// Will iterate the high level system function arguments, i.e Entitys and Queries
	template<typename T>
	static auto GetSingleAccessComponentAccesses()
	{
		return GetComponentTupleAccesses<typename T::ComponentTuple>();
	}

	template<typename Tuple, std::size_t... Indices>
	static auto GetComponentAccessesImpl(std::index_sequence<Indices...>)
	{
		Vector<ComponentAccess> result;
		(result.append(GetSingleAccessComponentAccesses<std::tuple_element_t<Indices, Tuple>>()), ...);
		return result;
	}

	template<typename Tuple, bool HasFloatArg>
	static auto GetComponentAccesses()
	{
		if constexpr (HasFloatArg)
		{
			constexpr std::size_t tupleSize = std::tuple_size_v<Tuple> - 1;
			return GetComponentAccessesImpl<Tuple>(std::make_index_sequence<tupleSize>{});
		}
		else
		{
			constexpr std::size_t tupleSize = std::tuple_size_v<Tuple>;
			return GetComponentAccessesImpl<Tuple>(std::make_index_sequence<tupleSize>{});
		}
	}

	// System functions
	template<typename Tuple, std::size_t... Indices>
	static auto GetRegistryViewImpl(std::index_sequence<Indices...>, entt::registry& registry)
	{
		return registry.view<std::remove_reference_t<std::tuple_element_t<Indices, Tuple>>...>();
	}

	template<typename Tuple>
	static auto GetRegistryView(entt::registry& registry)
	{
		constexpr std::size_t tupleSize = std::tuple_size_v<Tuple>;
		return GetRegistryViewImpl<Tuple>(std::make_index_sequence<tupleSize>{}, registry);
	}

	template<typename T, typename EntityView>
	static auto GetArgumentOfType(Volt::EntityScene& scene, EntityView& mainView, entt::entity entityId)
	{
		if constexpr (T::ConstructType == ECS::Type::Entity)
		{
			return T(scene.GetEntityHelperFromEntityHandle(entityId));
		}
		else if constexpr (T::ConstructType == ECS::Type::Query)
		{
			using ComponentTuple = typename T::ComponentViewTuple;
			return T(GetRegistryView<ComponentTuple>(scene.GetRegistry()), scene.GetRegistry());
		}
	}

	template<typename Tuple, std::size_t... Indices, typename EntityView>
	static auto GetSystemArgumentImpl(std::index_sequence<Indices...>, Volt::EntityScene& scene, EntityView& mainView, entt::entity entityId)
	{
		return std::tuple{ GetArgumentOfType<std::tuple_element_t<Indices, Tuple>>(scene, mainView, entityId)... };
	}

	template<typename Tuple, bool HasFloatArg, typename EntityView>
	static auto GetSystemArgument(Volt::EntityScene& scene, EntityView& mainView, entt::entity entityId)
	{
		if constexpr (HasFloatArg)
		{
			constexpr std::size_t tupleSize = std::tuple_size_v<Tuple> - 1;
			return GetSystemArgumentImpl<Tuple>(std::make_index_sequence<tupleSize>{}, scene, mainView, entityId);
		}
		else
		{
			constexpr std::size_t tupleSize = std::tuple_size_v<Tuple>;
			return GetSystemArgumentImpl<Tuple>(std::make_index_sequence<tupleSize>{}, scene, mainView, entityId);
		}
	}
};
