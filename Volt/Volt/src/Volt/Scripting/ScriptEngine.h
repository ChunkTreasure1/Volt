#pragma once

#include "Volt/Core/Base.h"

#include <Wire/Entity.h>
#include <Wire/WireGUID.h>

#include <unordered_map>

namespace Volt
{
	class ScriptBase;
	class ScriptEngine
	{
	public:
		static bool RegisterToEntity(Ref<ScriptBase> script, Wire::EntityId entity);
		static bool UnregisterFromEntity(WireGUID scriptGUID, Wire::EntityId entity);

		static bool IsScriptRegistered(WireGUID guid, Wire::EntityId entity);
		static Ref<ScriptBase> GetScript(Wire::EntityId entity, const WireGUID& guid);

		static std::vector<Ref<ScriptBase>> GetScriptsAttachedToEntity(Wire::EntityId entity);

		static void Clear();

	private:
		ScriptEngine() = delete;

		inline static std::unordered_map<Wire::EntityId, std::unordered_map<WireGUID, Ref<ScriptBase>>> s_scripts;
	};
}