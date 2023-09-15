#include "vtpch.h"
#include "MonoScriptEntity.h"
#include "MonoScriptEngine.h"
#include "MonoScriptClass.h"
#include "mono/jit/jit.h"

namespace Volt
{
	MonoScriptEntity::MonoScriptEntity(const entt::entity& id, const std::vector<uint64_t>& scripts, Ref<MonoScriptClass> klass)
	{
		myEntity = id;
		myMonoClass = klass;
		myHandle = MonoScriptEngine::InstantiateClass(UUID(static_cast<uint32_t>(id)), myMonoClass->GetClass());
		myUpdateTimersMethod = myMonoClass->GetMethod("UpdateTimers", 0);

		EntityParams params;
		params.id = id;
		params.scriptIds = MonoScriptUtils::CreateMonoArrayUInt64(scripts);

		void* args[2] = { &params.id, params.scriptIds };
		MonoScriptEngine::CallMethod(myHandle, MonoScriptEngine::GetEntityConstructor(), args);
	}

	MonoScriptEntity::~MonoScriptEntity()
	{
	}

	void MonoScriptEntity::UpdateTimers()
	{
		if (myUpdateTimersMethod)
		{
			MonoScriptEngine::CallMethod(myHandle, myUpdateTimersMethod);
		}
	}
}
