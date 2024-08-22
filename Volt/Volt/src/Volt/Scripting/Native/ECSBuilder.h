#pragma once

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/UUID.h>

#include <entt.hpp>

namespace Volt
{
	enum class GameLoop
	{
		Variable,
		Fixed
	};

	struct ECSSystem
	{
		std::function<void(entt::registry& registry)> systemFunc;
	};

	class ECSGameLoopContainer
	{
	public:
		template<typename Ret, typename... Args>
		void RegisterSystem(Ret(*func)(Args...))
		{
			using Traits = FunctionTraits<Ret(*)(Args...)>;
			using ArgumentTypes = typename Traits::ArgumentTypes;

			using ComponentView = std::tuple_element_t<0, ArgumentTypes>;
			using ComponentTuple = typename ComponentView::ComponentTuple;

			UUID64 id{};
			ECSSystem system{};
			system.systemFunc = [func](entt::registry& registry)
			{
				auto view = GetRegistryView<ComponentTuple>(registry);
				for (const auto& entity : view)
				{
					ComponentView componentView(GetComponentView<ComponentTuple>(view, entity));
					func(componentView);
				}
			};

			m_registeredSystems[id] = std::move(system);
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
			return view.get<std::remove_reference_t<std::tuple_element_t<Indices, Tuple>>...>(entity);
		}

		template<typename Tuple, typename EntityView>
		static auto GetComponentView(EntityView& view, entt::entity entity)
		{
			constexpr std::size_t tupleSize = std::tuple_size_v<Tuple>;
			return GetComponentViewImpl<Tuple>(std::make_index_sequence<tupleSize>{}, view, entity);
		}

		vt::map<UUID64, ECSSystem> m_registeredSystems;
	};

	struct ECSBuilder
	{
	public:
		ECSGameLoopContainer& GetGameLoop(GameLoop gameLoopType) { return m_gameLoops[gameLoopType]; }

	private:
		vt::map<GameLoop, ECSGameLoopContainer> m_gameLoops;
	};
}
