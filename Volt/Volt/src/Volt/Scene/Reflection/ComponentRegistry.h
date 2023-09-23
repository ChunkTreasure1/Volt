#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Scene/Reflection/ComponentReflection.h"

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
		static const ICommonTypeDesc* GetTypeDescFromGUID(const VoltGUID& guid);
		static std::string_view GetTypeNameFromGUID(const VoltGUID& guid);

		inline static const auto& GetRegistry() { return s_typeRegistry; }

		struct HelperFunctions
		{
			std::function<void(entt::registry&, entt::entity)> addComponent;
			std::function<void(entt::registry&, entt::entity)> removeComponent;
			std::function<bool(const entt::registry&, entt::entity)> hasComponent;
			std::function<void*(entt::registry&, entt::entity)> getComponent;
		};

		class Helpers
		{
		public:
			static void AddComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity);
			static void RemoveComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity);
			static const bool HasComponentWithGUID(const VoltGUID& guid, const entt::registry& registry, entt::entity entity);
			static void* GetComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity);

		private:
			Helpers() = default;
		};

	private:
		friend class Helpers;

		inline static std::unordered_map<VoltGUID, HelperFunctions> s_componentHelperFunctions;
		inline static std::unordered_map<VoltGUID, const ICommonTypeDesc*> s_typeRegistry;
		inline static std::unordered_map<std::string_view, VoltGUID> s_typeNameToGUIDMap;
		inline static std::unordered_map<VoltGUID, std::string_view> s_guidToTypeNameMap;
	};

	template<typename T>
	inline const bool ComponentRegistry::RegisterComponent()
	{
		static_assert(IsReflectedType<T>());

		const auto guid = GetTypeGUID<T>();

		if (s_typeRegistry.contains(guid))
		{
			return false;
		}

		const std::string_view name = entt::type_name<T>();

		s_typeRegistry[guid] = GetTypeDesc<T>();
		s_typeNameToGUIDMap[name] = guid;
		s_guidToTypeNameMap[guid] = name;

		auto& helpers = s_componentHelperFunctions[guid];
		helpers.addComponent = [](entt::registry& registry, entt::entity entity)
		{
			registry.emplace<T>(entity);
		};

		helpers.removeComponent = [](entt::registry& registry, entt::entity entity)
		{
			registry.remove<T>(entity);
		};

		helpers.hasComponent = [](const entt::registry& registry, entt::entity entity)
		{
			return registry.any_of<T>(entity);
		};

		helpers.getComponent = [](entt::registry& registry, entt::entity entity) -> void*
		{
			for (auto&& curr : registry.storage())
			{
				if (auto& storage = curr.second; curr.second.type().index() == entt::type_id<T>().index())
				{
					return storage.get(entity);
				}
			}

			return nullptr;
		};

		return true;
	}

	template<typename T>
	inline const bool ComponentRegistry::RegisterEnum()
	{
		static_assert(IsReflectedType<T>() && std::is_enum<T>::value);

		const auto guid = GetTypeGUID<T>();

		if (s_typeRegistry.contains(guid))
		{
			return false;
		}

		const std::string_view name = entt::type_name<T>();

		s_typeRegistry[guid] = GetTypeDesc<T>();
		s_typeNameToGUIDMap[name] = guid;
		s_guidToTypeNameMap[guid] = name;
		return true;
	}
}
