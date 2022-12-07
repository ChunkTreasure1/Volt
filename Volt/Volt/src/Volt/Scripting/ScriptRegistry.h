#pragma once

#include "Volt/Core/Base.h"
#include <Wire/WireGUID.h>

#include <unordered_map>

#define VT_REGISTER_SCRIPT(x) static bool x ## _entry = Volt::ScriptRegistry::Register(x::GetStaticGUID(), Volt::ScriptMetadata{ #x, x::Create});

namespace Volt
{
	class ScriptBase;
	class Entity;

	struct ScriptMetadata
	{
		using CreateMethod = Ref<ScriptBase>(*)(Entity entity);

		std::string name;
		CreateMethod createMethod = nullptr;
	};

	class ScriptRegistry
	{
	public:

		ScriptRegistry() = delete;

		static bool Register(const WireGUID& guid, const ScriptMetadata& data);
		static Ref<ScriptBase> Create(const WireGUID& guid, Entity ownerEntity);
		static Ref<ScriptBase> Create(const std::string& name, Entity ownerEntity);
		static const WireGUID GetGUIDFromName(const std::string& name);
		static const std::string GetNameFromGUID(const WireGUID& guid);

		inline static const std::unordered_map<WireGUID, ScriptMetadata>& GetRegistry() { return s_registry; }

	private:
		inline static std::unordered_map<WireGUID, ScriptMetadata> s_registry;
	};
}