#pragma once

#include "EntitySystem/Config.h"
#include "ECSAccessBuilder.h"

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/UUID.h>

enum class GameLoop
{
	Variable,
	Fixed
};

struct ECSSystem
{
	std::function<void(entt::registry& registry, float deltaTime)> systemFunc;
};

class ECSGameLoopContainer
{
public:
	ECSGameLoopContainer() = default;

	void Execute(float deltaTime);

	template<typename Ret, typename... Args>
	ECSSystem& RegisterSystem(Ret(*func)(Args...))
	{
		using Traits = FunctionTraits<Ret(*)(Args...)>;
		using ArgumentTypes = typename Traits::ArgumentTypes;

		// A system must have at least one argument...
		static_assert(std::tuple_size_v<ArgumentTypes> > 0);
		//static_assert(std::is_same_v<std::tuple_element_t<std::tuple_size_v<ArgumentTypes> - 1, ArgumentTypes>, float>);

		using ComponentView = std::tuple_element_t<0, ArgumentTypes>;

		// And the first should be an entity type
		static_assert(ComponentView::ConstructType == ECS::Type::Entity);

		using ComponentTuple = typename ComponentView::ComponentViewTuple;

		UUID64 id{};
		ECSSystem system{};
		system.systemFunc = [func](entt::registry& registry, float deltaTime)
		{
			auto view = GetRegistryView<ComponentTuple>(registry);
			for (const auto& entity : view)
			{
				auto arguments = GetSystemArgument<ArgumentTypes>(registry, view, entity);
				std::apply(func, arguments);
			}
		};

		m_registeredSystems[id] = std::move(system);
		return m_registeredSystems.at(id);
	}

private:
	template<typename T>
	struct FunctionTraits;

	template<typename Ret, typename... Args>
	struct FunctionTraits<Ret(*)(Args...)>
	{
		using ReturnType = Ret;
		using ArgumentTypes = std::tuple<Args...>;
	};

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

	template<typename Tuple, typename EntityView, std::size_t... Indices>
	static auto GetComponentViewImpl(std::index_sequence<Indices...>, EntityView& view, entt::entity entity)
	{
		// Note: We need to make sure that we always return a tuple
		if constexpr (sizeof... (Indices) > 1)
		{
			return view.get<std::remove_reference_t<std::tuple_element_t<Indices, Tuple>>...>(entity);
		}
		else
		{
			return std::tuple<std::tuple_element_t<Indices, Tuple>...>{ view.get<std::remove_reference_t<std::tuple_element_t<Indices, Tuple>>...>(entity) };
		}
	}

	template<typename Tuple, typename EntityView>
	static auto GetComponentView(EntityView& view, entt::entity entity)
	{
		constexpr std::size_t tupleSize = std::tuple_size_v<Tuple>;
		return GetComponentViewImpl<Tuple>(std::make_index_sequence<tupleSize>{}, view, entity);
	}

	template<typename T, typename EntityView>
	static auto GetArgumentOfType(entt::registry& registry, EntityView& mainView, entt::entity entityId)
	{
		if constexpr (T::ConstructType == ECS::Type::Entity)
		{
			using ComponentTuple = typename T::ComponentTuple;
			return T(GetComponentView<ComponentTuple>(mainView, entityId), entityId, registry);
		}
		else if constexpr (T::ConstructType == ECS::Type::Query)
		{
			using ComponentTuple = typename T::ComponentViewTuple;
			return T(GetRegistryView<ComponentTuple>(registry), registry);
		}
	}

	template<typename Tuple, std::size_t... Indices, typename EntityView>
	static auto GetSystemArgumentImpl(std::index_sequence<Indices...>, entt::registry& registry, EntityView& mainView, entt::entity entityId)
	{
		return std::tuple{ GetArgumentOfType<std::tuple_element_t<Indices, Tuple>>(registry, mainView, entityId)... };
	}

	template<typename Tuple, typename EntityView>
	static auto GetSystemArgument(entt::registry& registry, EntityView& mainView, entt::entity entityId)
	{
		constexpr std::size_t tupleSize = std::tuple_size_v<Tuple>;
		return GetSystemArgumentImpl<Tuple>(std::make_index_sequence<tupleSize>{}, registry, mainView, entityId);
	}

	vt::map<UUID64, ECSSystem> m_registeredSystems;
	entt::registry* m_registry = nullptr;
};

class ECSBuilder
{
public:
	ECSBuilder() = default;

	VTES_API ECSGameLoopContainer& GetGameLoop(GameLoop gameLoopType);

private:
	vt::map<GameLoop, ECSGameLoopContainer> m_gameLoops;
	entt::registry* m_registry = nullptr;
};
