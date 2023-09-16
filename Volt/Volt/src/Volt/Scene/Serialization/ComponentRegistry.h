#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Scene/Serialization/ComponentReflection.h"

#include <unordered_map>
#include <entt.hpp>

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

		static const ICommonTypeDesc* GetTypeDescFromName(std::string_view name);

	private:
		inline static std::unordered_map<VoltGUID, const ICommonTypeDesc*> m_typeRegistry;
		inline static std::unordered_map<std::string_view, VoltGUID> m_typeNameToGUIDMap;
	};

	template<typename T>
	inline const bool ComponentRegistry::RegisterComponent()
	{
		static_assert(IsReflectedType<T>());

		const auto guid = GetTypeGUID<T>();

		if (m_typeRegistry.contains(guid))
		{
			return false;
		}

		const std::string_view name = entt::type_name<T>();

		m_typeRegistry[guid] = GetTypeDesc<T>();
		m_typeNameToGUIDMap[name] = guid;

		return true;
	}

	template<typename T>
	inline const bool ComponentRegistry::RegisterEnum()
	{
		static_assert(IsReflectedType<T>() && std::is_enum<T>::value);

		const auto guid = GetTypeGUID<T>();

		if (m_typeRegistry.contains(guid))
		{
			return false;
		}

		const std::string_view name = entt::type_name<T>();

		m_typeRegistry[guid] = GetTypeDesc<T>();
		m_typeNameToGUIDMap[name] = guid;
		return true;
	}
}
