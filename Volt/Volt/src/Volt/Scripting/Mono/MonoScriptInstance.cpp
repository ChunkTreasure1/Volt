#include "vtpch.h"
#include "MonoScriptInstance.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptEntity.h"
#include "Volt/Scripting/Mono/MonoGCManager.h"

#include "Volt/Scripting/Mono/MonoScriptUtils.h"

#include <mono/metadata/object.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/profiler.h>
namespace Volt
{
	MonoScriptInstance::MonoScriptInstance(Ref<MonoScriptClass> monoClass, ScriptParams* params)
		: myMonoClass(monoClass)
	{
		myHandle = MonoScriptEngine::InstantiateClass(params->scriptId, myMonoClass->GetClass());

		myAwakeMethod = monoClass->GetMethod("OnAwake", 0);
		myCreateMethod = monoClass->GetMethod("OnCreate", 0);
		myDestroyMethod = monoClass->GetMethod("OnDestroy", 0);
		myUpdateMethod = monoClass->GetMethod("OnUpdate", 1);
		myOnAnimationEventMethod = monoClass->GetMethod("OnAnimationEvent", 2);
		myOnRenderUIMethod = monoClass->GetMethod("OnRenderUI", 0);

		myFixedUpdateMethod = monoClass->GetMethod("OnFixedUpdate", 1);
		myOnCollisionEnterMethod = monoClass->GetMethod("OnCollisionEnter", 1);
		myOnCollisionExitMethod = monoClass->GetMethod("OnCollisionExit", 1);
		myOnTriggerEnterMethod = monoClass->GetMethod("OnTriggerEnter", 1);
		myOnTriggerExitMethod = monoClass->GetMethod("OnTriggerExit", 1);

		myOnEnable = monoClass->GetMethod("OnEnable", 0);
		myOnDisable = monoClass->GetMethod("OnDisable", 0);

		void* args[2] = { params->entity, &params->scriptId };
		MonoScriptEngine::CallMethod(myHandle, MonoScriptEngine::GetScriptConstructor(), args);

		for (const auto& [name, field] : monoClass->GetFields())
		{
			myFieldNames.emplace_back(name);
		}
	}

	MonoScriptInstance::~MonoScriptInstance()
	{
	}

	void MonoScriptInstance::InvokeOnAwake()
	{
		if (myAwakeMethod)
		{
			MonoScriptEngine::CallMethod(myHandle, myAwakeMethod);
		}
	}

	void MonoScriptInstance::InvokeOnCreate()
	{
		if (myCreateMethod)
		{
			MonoScriptEngine::CallMethod(myHandle, myCreateMethod);
		}
	}

	void MonoScriptInstance::InvokeOnDestroy()
	{
		if (myDestroyMethod)
		{
			MonoScriptEngine::CallMethod(myHandle, myDestroyMethod);
		}
	}

	void MonoScriptInstance::InvokeOnUpdate(float deltaTime)
	{
		if (myUpdateMethod)
		{
			void* param = &deltaTime;
			MonoScriptEngine::CallMethod(myHandle, myUpdateMethod, &param);
		}
	}

	void MonoScriptInstance::InvokeOnFixedUpdate(float deltaTime)
	{
		if (myFixedUpdateMethod)
		{
			void* param = &deltaTime;
			MonoScriptEngine::CallMethod(myHandle, myFixedUpdateMethod, &param);
		}
	}

	void MonoScriptInstance::InvokeOnAnimationEvent(const std::string& eventName, uint32_t frame)
	{
		if (myOnAnimationEventMethod)
		{
			auto monoString = MonoScriptUtils::GetMonoStringFromString(eventName);

			void* params[] =
			{
				monoString,
				&frame
			};

			MonoScriptEngine::CallMethod(myHandle, myOnAnimationEventMethod, params);
		}
	}

	void MonoScriptInstance::InvokeOnRenderUI()
	{
		if (myOnRenderUIMethod)
		{
			MonoScriptEngine::CallMethod(myHandle, myOnRenderUIMethod);
		}
	}

	void MonoScriptInstance::InvokeOnCollisionEnter(Volt::Entity entity)
	{
		if (myOnCollisionEnterMethod)
		{
			auto entityInstance = MonoScriptEngine::GetEntityFromId(entity.GetId());
			if (!entityInstance)
			{
				entityInstance = MonoScriptEngine::GetOrCreateMonoEntity(entity.GetId());
			}

			auto monoEntity = MonoGCManager::GetObjectFromHandle(entityInstance->GetHandle());

			void* param = monoEntity;
			MonoScriptEngine::CallMethod(myHandle, myOnCollisionEnterMethod, &param);
		}
	}

	void MonoScriptInstance::InvokeOnCollisionExit(Volt::Entity entity)
	{
		if (myOnCollisionExitMethod)
		{
			auto entityInstance = MonoScriptEngine::GetEntityFromId(entity.GetId());
			if (!entityInstance)
			{
				entityInstance = MonoScriptEngine::GetOrCreateMonoEntity(entity.GetId());
			}

			auto monoEntity = MonoGCManager::GetObjectFromHandle(entityInstance->GetHandle());

			void* param = monoEntity;
			MonoScriptEngine::CallMethod(myHandle, myOnCollisionExitMethod, &param);
		}
	}

	void MonoScriptInstance::InvokeOnTriggerEnter(Volt::Entity entity)
	{
		if (myOnTriggerEnterMethod)
		{
			auto entityInstance = MonoScriptEngine::GetEntityFromId(entity.GetId());
			if (!entityInstance)
			{
				entityInstance = MonoScriptEngine::GetOrCreateMonoEntity(entity.GetId());
			}

			auto monoEntity = MonoGCManager::GetObjectFromHandle(entityInstance->GetHandle());

			void* param = monoEntity;
			MonoScriptEngine::CallMethod(myHandle, myOnTriggerEnterMethod, &param);
		}
	}

	void MonoScriptInstance::InvokeOnTriggerExit(Volt::Entity entity)
	{
		if (myOnTriggerExitMethod)
		{
			auto entityInstance = MonoScriptEngine::GetEntityFromId(entity.GetId());
			if (!entityInstance)
			{
				entityInstance = MonoScriptEngine::GetOrCreateMonoEntity(entity.GetId());
			}

			auto monoEntity = MonoGCManager::GetObjectFromHandle(entityInstance->GetHandle());
			void* param = monoEntity;
			MonoScriptEngine::CallMethod(myHandle, myOnTriggerExitMethod, &param);
		}
	}

	void MonoScriptInstance::InvokeOnEnable()
	{
		if (myOnEnable)
		{
			MonoScriptEngine::CallMethod(myHandle, myOnEnable);
		}
	}

	void MonoScriptInstance::InvokeOnDisable()
	{
		if (myOnDisable)
		{
			MonoScriptEngine::CallMethod(myHandle, myOnDisable);
		}
	}

	void MonoScriptInstance::SetField(const std::string& name, const void* value)
	{
		SetFieldInternal(name, value);
	}

	bool MonoScriptInstance::GetFieldInternal(const std::string& name, void* outData)
	{
		auto instance = MonoGCManager::GetObjectFromHandle(myHandle);
		if (!instance)
		{
			return false;
		}

		const auto& fields = myMonoClass->GetFields();
		if (!fields.contains(name))
		{
			return false;
		}

		const auto& field = fields.at(name);
		mono_field_get_value(instance, field.fieldPtr, outData);

		return true;
	}

	bool MonoScriptInstance::SetFieldInternal(const std::string& name, const void* value)
	{
		auto instance = MonoGCManager::GetObjectFromHandle(myHandle);
		if (!instance)
		{
			return false;
		}

		const auto& fields = myMonoClass->GetFields();
		if (!fields.contains(name))
		{
			return false;
		}

		const auto& field = fields.at(name);
		mono_field_set_value(instance, field.fieldPtr, (void*)value);
		return true;
	}
}
