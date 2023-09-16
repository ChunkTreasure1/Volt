#include "vtpch.h"
#include "ComponentRegistry.h"

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
}
