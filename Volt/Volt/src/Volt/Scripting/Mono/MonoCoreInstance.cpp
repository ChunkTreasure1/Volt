#include "vtpch.h"
#include "MonoCoreInstance.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoGCManager.h"

namespace Volt
{
	MonoCoreInstance::MonoCoreInstance(Ref<MonoScriptClass> monoClass)
		: myMonoClass(monoClass)
	{
		myHandle = MonoScriptEngine::InstantiateClass(myUUID, myMonoClass->GetClass());
	}
	
	MonoCoreInstance::~MonoCoreInstance()
	{
	}
	
	void MonoCoreInstance::CallMethod(const std::string& methodName)
	{
		auto* method = myMonoClass->GetMethod(methodName, 0);
		if (!method)
		{
			return;
		}

		MonoScriptEngine::CallMethod(myHandle, method);
	}
}
