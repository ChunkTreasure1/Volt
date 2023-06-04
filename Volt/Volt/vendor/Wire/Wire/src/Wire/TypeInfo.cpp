#include "TypeInfo.h"

namespace Wire
{
	inline static TypeRegistry::TypeInfo s_nullTypeInfo;

	const TypeRegistry::TypeInfo& TypeRegistry::GetTypeInfoFromPrettyName(const std::string& name)
	{
		if (!GetTypeRegistry().contains(name))
		{
			return s_nullTypeInfo;
		}

		return GetTypeRegistry().at(name);
	}

	const TypeRegistry::TypeInfo& TypeRegistry::GetTypeInfoFromTypeName(const std::string& name)
	{
		for (const auto& [prettyName, typeInfo] : GetTypeRegistry())
		{
			if (typeInfo.typeName == name)
			{
				return typeInfo;
			}
		}

		return s_nullTypeInfo;
	}

	const TypeRegistry::TypeInfo& TypeRegistry::GetTypeInfoFromTypeIndex(const std::type_index& typeIndex)
	{
		if (!GetNameRegistry().contains(typeIndex))
		{
			return s_nullTypeInfo;
		}

		const auto& name = GetNameRegistry().at(typeIndex);
		if (!GetTypeRegistry().contains(name))
		{
			return s_nullTypeInfo;
		}

		return GetTypeRegistry().at(name);
	}
}