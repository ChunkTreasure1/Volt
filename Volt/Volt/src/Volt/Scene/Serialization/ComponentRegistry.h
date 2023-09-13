#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Scene/Serialization/ComponentReflection.h"

#include <unordered_map>

#define REGISTER_COMPONENT(compType) inline static bool _comp_registered = Volt::ComponentRegistry::RegisterComponent<compType>()

namespace Volt
{
	class ComponentRegistry
	{
	public:
		template<typename T>
		static const bool RegisterComponent();

	private:
		inline static std::unordered_map<VoltGUID, Scope<IComponentDesc>> m_registry;
	};

	template<typename T>
	inline const bool ComponentRegistry::RegisterComponent()
	{
		Scope<ComponentDesc<T>> reflectionType = CreateScope<ComponentDesc<T>>();
		T::Reflect(*reflectionType);

		if (m_registry.contains(reflectionType->GetGUID()))
		{
			return false;
		}

		m_registry[reflectionType->GetGUID()] = std::move(reflectionType);
		return true;
	}
}
