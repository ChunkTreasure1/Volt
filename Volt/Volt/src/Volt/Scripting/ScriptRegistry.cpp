#include "vtpch.h"
#include "ScriptRegistry.h"

#include "Volt/Scene/Entity.h"

namespace Volt
{
	bool ScriptRegistry::Register(const WireGUID& guid, const ScriptMetadata& data)
	{
		if (const auto& it = s_registry.find(guid); it == s_registry.end())
		{
			s_registry[guid] = data;
			return true;
		}

		return false;
	}

	Ref<ScriptBase> ScriptRegistry::Create(const WireGUID& guid, Entity ownerEntity)
	{
		if (const auto& it = s_registry.find(guid); it != s_registry.end())
		{
			return it->second.createMethod(ownerEntity);
		}

		return nullptr;
	}

	Ref<ScriptBase> ScriptRegistry::Create(const std::string& name, Entity ownerEntity)
	{
		for (const auto& [guid, info] : s_registry)
		{
			if (info.name == name)
			{
				return Create(guid, ownerEntity);
			}
		}

		return nullptr;
	}

	const WireGUID ScriptRegistry::GetGUIDFromName(const std::string& name)
	{
		for (const auto& [guid, info] : s_registry)
		{
			if (info.name == name)
			{
				return guid;
			}
		}

		return WireGUID::Null();
	}

	const std::string ScriptRegistry::GetNameFromGUID(const WireGUID& guid)
	{
		if (s_registry.find(guid) == s_registry.end())
		{
			return "Null";
		}

		return s_registry.at(guid).name;
	}
}