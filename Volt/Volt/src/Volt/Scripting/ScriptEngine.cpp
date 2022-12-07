#include "vtpch.h"
#include "ScriptEngine.h"

#include "Volt/Scripting/ScriptBase.h"
#include "Volt/Log/Log.h"

namespace Volt
{
	bool ScriptEngine::RegisterToEntity(Ref<ScriptBase> script, Wire::EntityId entity)
	{
		auto& entScripts = s_scripts[entity];
		if (const auto& it = entScripts.find(script->GetGUID()); it != entScripts.end())
		{
			VT_CORE_ERROR("Script has already been registered to this entity!");
			return false;
		}

		entScripts[script->GetGUID()] = script;
		return true;
	}

	bool ScriptEngine::UnregisterFromEntity(WireGUID scriptGUID, Wire::EntityId entity)
	{
		auto& entScripts = s_scripts[entity];
		auto it = entScripts.find(scriptGUID);
		if (it == entScripts.end())
		{
			VT_CORE_ERROR("Script is not registered to this entity!");
			return false;
		}

		entScripts.erase(it);
		return true;
	}

	bool ScriptEngine::IsScriptRegistered(WireGUID guid, Wire::EntityId entity)
	{
		return s_scripts[entity].find(guid) != s_scripts[entity].end();
	}

	void ScriptEngine::Clear()
	{
		s_scripts.clear();
	}

	Ref<ScriptBase> ScriptEngine::GetScript(Wire::EntityId entity, const WireGUID& guid)
	{
		const auto& entScripts = s_scripts[entity];

		if (const auto& it = entScripts.find(guid); it != entScripts.end())
		{
			return it->second;
		}

		return nullptr;
	}

	std::vector<Ref<ScriptBase>> ScriptEngine::GetScriptsAttachedToEntity(Wire::EntityId entity)
	{
		std::vector<Ref<ScriptBase>> result;
		if (s_scripts.find(entity) == s_scripts.end())
		{
			return result;
		}

		const auto& scripts = s_scripts.at(entity);
		for (const auto& [guid, script] : scripts)
		{
			result.emplace_back(script);
		}

		return result;
	}
}