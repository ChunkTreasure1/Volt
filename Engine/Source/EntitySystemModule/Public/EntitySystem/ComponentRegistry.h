#pragma once

#include "Config.h"
#include "ComponentReflection.h"

#include <unordered_map>
#include <entt.hpp>

namespace Volt
{
	class VTES_API ComponentRegistry
	{
	public:
		template<typename T>
		const bool RegisterComponent();

		template<typename T>
		const bool RegisterEnum();

		const ICommonTypeDesc* GetTypeDescFromName(std::string_view name);
		const ICommonTypeDesc* GetTypeDescFromGUID(const VoltGUID& guid);
		std::string_view GetTypeNameFromGUID(const VoltGUID& guid);
		const VoltGUID GetGUIDFromTypeName(std::string_view typeName);

		inline const auto& GetRegistry() { return m_typeRegistry; }

		struct HelperFunctions
		{
			std::function<void(entt::registry&, entt::entity)> addComponent;
			std::function<void(entt::registry&, entt::entity)> removeComponent;
			std::function<bool(const entt::registry&, entt::entity)> hasComponent;
			std::function<void*(entt::registry&, entt::entity)> getComponent;
			std::function<void(entt::registry&)> setupOnCreate;
			std::function<void(entt::registry&)> setupOnDestroy;
		};

		class Helpers
		{
		public:
			VTES_API static void AddComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity);
			VTES_API static void RemoveComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity);
			VTES_API static const bool HasComponentWithGUID(const VoltGUID& guid, const entt::registry& registry, entt::entity entity);
			VTES_API static void* GetComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity);
			VTES_API static void SetupComponentCallbacks(entt::registry& registry);

		private:
			Helpers() = default;
		};

	private:
		friend class Helpers;

		template<typename T>
		static void OnConstructComponent(entt::registry& registry, entt::entity entity);

		template<typename T>
		static void OnDestructComponent(entt::registry& registry, entt::entity entity);

		std::unordered_map<VoltGUID, HelperFunctions> m_componentHelperFunctions;
		std::unordered_map<VoltGUID, const ICommonTypeDesc*> m_typeRegistry;
		std::unordered_map<std::string_view, VoltGUID> m_typeNameToGUIDMap;
		std::unordered_map<VoltGUID, std::string_view> m_guidToTypeNameMap;
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
		m_guidToTypeNameMap[guid] = name;

		auto& helpers = m_componentHelperFunctions[guid];
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
				if (auto& storage = curr.second; curr.second.type().name() == entt::type_id<T>().name())
				{
					return storage.get(entity);
				}
			}

			return nullptr;
		};

		helpers.setupOnCreate = [](entt::registry& registry)
		{
			registry.on_construct<T>().template connect<&ComponentRegistry::OnConstructComponent<T>>();
		};

		helpers.setupOnDestroy = [](entt::registry& registry)
		{
			registry.on_destroy<T>().template connect<&ComponentRegistry::OnDestructComponent<T>>();
		};

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
		m_guidToTypeNameMap[guid] = name;
		return true;
	}

	template<typename T>
	inline void ComponentRegistry::OnConstructComponent(entt::registry& registry, entt::entity entity)
	{
		const auto* typeDesc = GetTypeDesc<T>();
		const IComponentTypeDesc* compDesc = reinterpret_cast<const IComponentTypeDesc*>(typeDesc);
		compDesc->OnCreate(registry, entity);
	}

	template<typename T>
	inline void ComponentRegistry::OnDestructComponent(entt::registry& registry, entt::entity entity)
	{
		const auto* typeDesc = GetTypeDesc<T>();
		const IComponentTypeDesc* compDesc = reinterpret_cast<const IComponentTypeDesc*>(typeDesc);
		compDesc->OnDestroy(registry, entity);
	}
}

extern VTES_API Volt::ComponentRegistry g_componentRegistry;

VT_INLINE Volt::ComponentRegistry& GetComponentRegistry()
{
	return g_componentRegistry;
}

#define REGISTER_COMPONENT(compType) inline static bool compType ## _comp_registered = ::GetComponentRegistry().RegisterComponent<compType>()
#define REGISTER_ENUM(enumType) inline static bool enumType ## _enum_registered = ::GetComponentRegistry().RegisterEnum<enumType>()
