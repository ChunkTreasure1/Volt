#include "vtpch.h"
#include "MonoScriptInstance.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include <mono/jit/jit.h>

namespace Volt
{
	MonoScriptInstance::MonoScriptInstance(Ref<MonoScriptClass> monoClass, Wire::EntityId entityId)
		: myMonoClass(monoClass)
	{
		myInstance = monoClass->Instantiate();

		myCreateMethod = monoClass->GetMethod("OnCreate", 0);
		myDestroyMethod = monoClass->GetMethod("OnDestroy", 0);
		myUpdateMethod = monoClass->GetMethod("OnUpdate", 1);
	
		void* idParam = &entityId;
		monoClass->InvokeMethod(myInstance, MonoScriptEngine::GetEntityConstructor(), &idParam);

		for (const auto& [name, field] : monoClass->GetFields())
		{
			myFieldNames.emplace_back(name);
		}
	}

	void MonoScriptInstance::InvokeOnCreate()
	{
		if (myCreateMethod)
		{
			myMonoClass->InvokeMethod(myInstance, myCreateMethod);
		}
	}

	void MonoScriptInstance::InvokeOnDestroy()
	{
		if (myDestroyMethod)
		{
			myMonoClass->InvokeMethod(myInstance, myDestroyMethod);
		}
	}

	void MonoScriptInstance::InvokeOnUpdate(float deltaTime)
	{
		if (myUpdateMethod)
		{
			void* param = &deltaTime;
			myMonoClass->InvokeMethod(myInstance, myUpdateMethod, &param);
		}
	}

	void MonoScriptInstance::SetField(const std::string& name, const void* value)
	{
		SetFieldInternal(name, value);
	}

	bool MonoScriptInstance::GetFieldInternal(const std::string& name, void* outData)
	{
		const auto& fields = myMonoClass->GetFields();

		if (!fields.contains(name))
		{
			return false;
		}

		const auto& field = fields.at(name);
		mono_field_get_value(myInstance, field.fieldPtr, outData);

		return true;
	}

	bool MonoScriptInstance::SetFieldInternal(const std::string& name, const void* value)
	{
		const auto& fields = myMonoClass->GetFields();

		if (!fields.contains(name))
		{
			return false;
		}

		const auto& field = fields.at(name);
		mono_field_set_value(myInstance, field.fieldPtr, (void*)value);
		return true;
	}
}