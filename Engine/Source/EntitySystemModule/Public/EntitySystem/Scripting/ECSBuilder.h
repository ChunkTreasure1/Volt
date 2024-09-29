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
	ECSSystemFunc m_systemFunc;
	ECSExecutionOrder m_executionOrder;

	UUID64 m_id = 0;
};

class ECSGameLoopContainer
{
public:
	ECSGameLoopContainer() = default;

	void VTES_API Execute(Volt::EntityScene& scene, float deltaTime);
	void VTES_API Compile();

	template<typename Ret, typename... Args>
	ECSSystem& RegisterSystem(Ret(*func)(Args...))
	{
		using Traits = FunctionTraits<Ret(*)(Args...)>;
		using ArgumentTypes = typename Traits::ArgumentTypes;

		// A system must have at least one argument...
		static_assert(std::tuple_size_v<ArgumentTypes> > 0);
		static_assert(std::is_same_v<std::tuple_element_t<std::tuple_size_v<ArgumentTypes> - 1, ArgumentTypes>, float>);

		using ComponentView = std::tuple_element_t<0, ArgumentTypes>;

		// And the first should be an entity type
		static_assert(ComponentView::ConstructType == ECS::Type::Entity);

		using ComponentTuple = typename ComponentView::ComponentViewTuple;

		UUID64 id{};

		auto systemFunc = [func](Volt::EntityScene& scene, float deltaTime)
		{
			auto view = GetRegistryView<ComponentTuple>(scene.GetRegistry());
			auto deltaTimeTuple = std::tuple{ deltaTime };

			for (const auto& entity : view)
			{
				auto arguments = GetSystemArgument<ArgumentTypes>(scene, view, entity);
				auto finalArguments = std::tuple_cat(arguments, deltaTimeTuple);
				std::apply(func, finalArguments);
			}
		};

		m_registeredSystems[id] = ECSSystem(id, std::move(systemFunc));;
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

	template<typename Tuple, typename EntityView>
	static auto GetSystemArgument(Volt::EntityScene& scene, EntityView& mainView, entt::entity entityId)
	{
		constexpr std::size_t tupleSize = std::tuple_size_v<Tuple> - 1;
		return GetSystemArgumentImpl<Tuple>(std::make_index_sequence<tupleSize>{}, scene, mainView, entityId);
	}

	vt::map<UUID64, ECSSystem> m_registeredSystems;
	Vector<Vector<UUID64>> m_executionBuckets;
};

class VTES_API ECSBuilder
{
public:
	ECSBuilder() = default;

	void Compile();

	ECSGameLoopContainer& GetGameLoop(GameLoop gameLoopType);

private:
	std::unordered_map<GameLoop, ECSGameLoopContainer> m_gameLoops;
};
