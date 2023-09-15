#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Scene/Serialization/ComponentReflection.h"

#include <unordered_map>

#define REGISTER_COMPONENT(compType) inline static bool compType ## _comp_registered = Volt::ComponentRegistry::RegisterComponent<compType>()
#define REGISTER_ENUM(enumType) inline static bool enumType ## _enum_registered = Volt::ComponentRegistry::RegisterEnum<enumType>()

namespace Volt
{
	class ComponentRegistry
	{
	public:
		template<typename T>
		static const bool RegisterComponent();

		template<typename T>
		static const bool RegisterEnum();

	private:
		inline static std::unordered_map<VoltGUID, const ICommonTypeDesc*> m_componentRegistry;
		inline static std::unordered_map<VoltGUID, const IEnumTypeDesc*> m_enumRegistry;
	};

	template<typename T>
	inline const bool ComponentRegistry::RegisterComponent()
	{
		static_assert(IsReflectedType<T>());

		const auto guid = GetTypeGUID<T>();

		if (m_componentRegistry.contains(guid))
		{
			return false;
		}

		m_componentRegistry[guid] = GetTypeDesc<T>();
		return true;
	}

	template<typename T>
	inline const bool ComponentRegistry::RegisterEnum()
	{
		static_assert(IsReflectedType<T>() && std::is_enum<T>::value);

		const auto guid = GetTypeGUID<T>();

		if (m_enumRegistry.contains(guid))
		{
			return false;
		}

		m_enumRegistry[guid] = GetTypeDesc<T>();
		return true;
	}
}
