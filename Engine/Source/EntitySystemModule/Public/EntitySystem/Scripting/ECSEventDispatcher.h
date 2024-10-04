#pragma once

#include <EntitySystem/Scripting/ECSSystem.h>

#include <EventSystem/Event.h>
#include <EventSystem/EventListener.h>

#include <CoreUtilities/TypeTraits/TypeIndex.h>

class ECSEventDispatcher : public Volt::EventListener
{
public:
	ECSEventDispatcher();

	template<Volt::IsEvent T, typename... Filter, typename F>
	void RegisterListenerSystem(const F& func)
	{
		constexpr bool EventHasGetEntitiesFunc = HasGetEntitiesFunc<T, Vector<Volt::EntityHelper>(void)>::value;

		const TypeTraits::TypeIndex typeIndex = TypeTraits::TypeIndex::FromType<T>();

		if (!m_registeredListeners.contains(typeIndex))
		{
			RegisterListener<T>([this, typeIndex](T& event) -> bool
			{
				for (auto& listenerInfo : m_registeredListeners.at(typeIndex))
				{
					if constexpr (EventHasGetEntitiesFunc)
					{
						bool isValid = false;

						const auto entities = event.GetEntities();
						for (const auto& entity : entities)
						{
							for (const auto& filterAccess : listenerInfo.componentAccesses)
							{
								if (entity.HasComponent(filterAccess.componentGUID))
								{
									isValid = true;
									break;
								}
							}

							if (isValid)
							{
								break;
							}
						}

						if (isValid)
						{
							listenerInfo.func(&event);
						}
					}
					else
					{
						listenerInfo.func(&event);
					}
				}

				return false;
			});
		}

		auto eventFunc = [func](void* eventPtr)
		{
			T& eventRef = *reinterpret_cast<T*>(eventPtr);
			func(eventRef);
		};

		using FilterTuple = std::tuple<Filter...>;

		ECSEventListenerInfo listenerInfo{};

		if constexpr (std::tuple_size_v<FilterTuple> > 0)
		{
			ECSSystemRegisterer<void, Filter...> systemRegisterer;
			listenerInfo.componentAccesses = systemRegisterer.GetSystemComponentAccesses();
		}

		listenerInfo.func = eventFunc;

		m_registeredListeners[typeIndex].emplace_back(std::move(listenerInfo));
	}

	void Clear();

private:
	using EventFunc = std::function<void(void*)>;

	template<typename, typename T, typename = std::void_t<>>
	struct HasGetEntitiesFunc : std::false_type { };

	template <typename C, typename Ret, typename... Args>
	struct HasGetEntitiesFunc<C, Ret(Args...), std::void_t<decltype(std::declval<C>().GetEntities(std::declval<Args>()...))>>
		: std::is_same<decltype(std::declval<C>().GetEntities(std::declval<Args>()...)), Ret>
	{};

	struct ECSEventListenerInfo
	{
		EventFunc func;
		Vector<ComponentAccess> componentAccesses;
	};

	vt::map<TypeTraits::TypeIndex, Vector<ECSEventListenerInfo>> m_registeredListeners;
};
