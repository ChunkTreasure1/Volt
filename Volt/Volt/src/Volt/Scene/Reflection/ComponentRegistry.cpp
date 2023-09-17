#include "vtpch.h"
#include "ComponentRegistry.h"

namespace Volt
{
	const ICommonTypeDesc* Volt::ComponentRegistry::GetTypeDescFromName(std::string_view name)
	{
		if (!s_typeNameToGUIDMap.contains(name))
		{
			return nullptr;
		}

		return s_typeRegistry.at(s_typeNameToGUIDMap.at(name));
	}

	const ICommonTypeDesc* ComponentRegistry::GetTypeDescFromGUID(const VoltGUID& guid)
	{
		if (!s_typeRegistry.contains(guid))
		{
			return nullptr;
		}

		return s_typeRegistry.at(guid);
	}

	std::string_view ComponentRegistry::GetTypeNameFromGUID(const VoltGUID& guid)
	{
		return s_guidToTypeNameMap.at(guid);
	}

	void ComponentRegistry::Helpers::AddComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity)
	{
		if (!ComponentRegistry::s_componentHelperFunctions.contains(guid))
		{
			return;
		}

		ComponentRegistry::s_componentHelperFunctions.at(guid).addComponent(registry, entity);
	}

	const bool ComponentRegistry::Helpers::HasComponentWithGUID(const VoltGUID& guid, const entt::registry& registry, entt::entity entity)
	{
		if (!ComponentRegistry::s_componentHelperFunctions.contains(guid))
		{
			return false;
		}

		return ComponentRegistry::s_componentHelperFunctions.at(guid).hasComponent(registry, entity);
	}

	void* ComponentRegistry::Helpers::GetComponentWithGUID(const VoltGUID& guid, entt::registry& registry, entt::entity entity)
	{
		if (!ComponentRegistry::s_componentHelperFunctions.contains(guid))
		{
			return nullptr;
		}

		return ComponentRegistry::s_componentHelperFunctions.at(guid).getComponent(registry, entity);
	}
}
