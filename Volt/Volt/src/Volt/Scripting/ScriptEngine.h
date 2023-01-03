#pragma once

#include "Volt/Core/Base.h"

#include <Wire/Entity.h>
#include <Wire/WireGUID.h>

#include <unordered_map>

namespace Volt
{
	class Script;
	class ScriptEngine
	{
	public:
		static bool RegisterToEntity(Ref<Script> script, Wire::EntityId entity);
		static bool UnregisterFromEntity(WireGUID scriptGUID, Wire::EntityId entity);

		static bool IsScriptRegistered(WireGUID guid, Wire::EntityId entity);
		static Ref<Script> GetScript(Wire::EntityId entity, const WireGUID& guid);

		static std::vector<Ref<Script>> GetScriptsAttachedToEntity(Wire::EntityId entity);

		static void Clear();

	private:
		ScriptEngine() = delete;

		inline static std::unordered_map<Wire::EntityId, std::unordered_map<WireGUID, Ref<Script>>> s_scripts;
	};
}