#include "espch.h"
#include "ComponentRegistry.h"

Volt::ComponentRegistry g_componentRegistry;

namespace Volt
{
	const ICommonTypeDesc* Volt::ComponentRegistry::GetTypeDescFromName(std::string_view name)
	{
		if (!m_typeNameToGUIDMap.contains(name))
		{
			return nullptr;
		}

		return m_typeRegistry.at(m_typeNameToGUIDMap.at(name));
	}

	const ICommonTypeDesc* ComponentRegistry::GetTypeDescFromGUID(const VoltGUID& guid)
	{
		if (!m_typeRegistry.contains(guid))
		{
			return nullptr;
		}

		return m_typeRegistry.at(guid);
	}

	std::string_view ComponentRegistry::GetTypeNameFromGUID(const VoltGUID& guid)
	{
		return m_guidToTypeNameMap.at(guid);
	}

	const VoltGUID ComponentRegistry::GetGUIDFromTypeName(std::string_view typeName)
	{
		return m_typeNameToGUIDMap.at(typeName);
	}

	void ComponentRegistry::Helpers::AddComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity)
	{
		ComponentRegistry& componentRegistry = GetComponentRegistry();
		VT_ENSURE(componentRegistry.m_componentHelperFunctions.contains(guid));
		componentRegistry.m_componentHelperFunctions.at(guid).addComponent(registry, entity);
	}

	void ComponentRegistry::Helpers::RemoveComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity)
	{
		ComponentRegistry& componentRegistry = GetComponentRegistry();
		VT_ENSURE(componentRegistry.m_componentHelperFunctions.contains(guid));
		componentRegistry.m_componentHelperFunctions.at(guid).removeComponent(registry, entity);
	}

	const bool ComponentRegistry::Helpers::HasComponentWithGUID(const VoltGUID& guid, const entt::registry& registry, entt::entity entity)
	{
		ComponentRegistry& componentRegistry = GetComponentRegistry();
		VT_ENSURE(componentRegistry.m_componentHelperFunctions.contains(guid));
		return componentRegistry.m_componentHelperFunctions.at(guid).hasComponent(registry, entity);
	}

	void* ComponentRegistry::Helpers::GetComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity)
	{
		ComponentRegistry& componentRegistry = GetComponentRegistry();
		VT_ENSURE(componentRegistry.m_componentHelperFunctions.contains(guid));
		return componentRegistry.m_componentHelperFunctions.at(guid).getComponent(registry, entity);
	}

	void ComponentRegistry::Helpers::SetupComponentCallbacks(entt::registry& registry)
	{
		ComponentRegistry& componentRegistry = GetComponentRegistry();
		for (auto& [uuid, helpers] : componentRegistry.m_componentHelperFunctions)
		{
			helpers.setupOnCreate(registry);
			helpers.setupOnDestroy(registry);
		}
	}
}
