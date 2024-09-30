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
		const TypeTraits::TypeIndex typeIndex = TypeTraits::TypeIndex::FromType<T>();

		if (!m_registeredListeners.contains(typeIndex))
		{
			RegisterListener<T>([this, typeIndex](T& event) -> bool
			{
				// #TODO_Ivar: Add filtering based on whether event has correct entity components. (Not sure how)
				for (auto& listenerInfo : m_registeredListeners.at(typeIndex))
				{
					listenerInfo.func(&event);
				}

				return false;
			});
		}

		auto eventFunc = [func](void* eventPtr)
		{
			T& eventRef = *reinterpret_cast<T*>(eventPtr);
			func(eventRef);
		};

		ECSSystemRegisterer<void, Filter...> systemRegisterer;

		ECSEventListenerInfo listenerInfo{};
		listenerInfo.componentAccesses = systemRegisterer.GetSystemComponentAccesses();
		listenerInfo.func = eventFunc;

		m_registeredListeners[typeIndex].emplace_back(std::move(listenerInfo));
	}

private:
	using EventFunc = std::function<void(void*)>;

	struct ECSEventListenerInfo
	{
		EventFunc func;
		Vector<ComponentAccess> componentAccesses;
	};

	vt::map<TypeTraits::TypeIndex, Vector<ECSEventListenerInfo>> m_registeredListeners;
};
