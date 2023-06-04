#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/Buffer.h"
#include "Volt/Scene/Entity.h"
#include <Wire/Wire.h>

extern "C"
{
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Volt
{
	struct ScriptParams
	{
		MonoObject* entity;
		UUID scriptId;
	};

	using GCHandle = void*;

	class MonoScriptClass;
	class MonoScriptInstance
	{
	public:
		MonoScriptInstance(Ref<MonoScriptClass> monoClass, ScriptParams* params);
		~MonoScriptInstance();

		void InvokeOnAwake();
		void InvokeOnCreate();
		void InvokeOnDestroy();
		void InvokeOnUpdate(float deltaTime);
		void InvokeOnFixedUpdate(float deltaTime);
		void InvokeOnAnimationEvent(const std::string& eventName, uint32_t frame);

		void InvokeOnRenderUI();

		void InvokeOnCollisionEnter(Volt::Entity entity);
		void InvokeOnCollisionExit(Volt::Entity entity);

		void InvokeOnTriggerEnter(Volt::Entity entity);
		void InvokeOnTriggerExit(Volt::Entity entity);

		void InvokeOnEnable();
		void InvokeOnDisable();

		void SetField(const std::string& name, const void* value);

		template<typename T>
		const T GetField(const std::string& name);

		inline const Ref<MonoScriptClass> GetClass() const { return myMonoClass; }
		inline const GCHandle GetHandle() const { return myHandle; }

	private:
		bool GetFieldInternal(const std::string& name, void* outData);
		bool SetFieldInternal(const std::string& name, const void* value);

		Ref<MonoScriptClass> myMonoClass;

		MonoMethod* myIdConstructorMethod = nullptr;

		MonoMethod* myAwakeMethod = nullptr;
		MonoMethod* myCreateMethod = nullptr;
		MonoMethod* myDestroyMethod = nullptr;
		MonoMethod* myUpdateMethod = nullptr;
		MonoMethod* myFixedUpdateMethod = nullptr;
		MonoMethod* myOnAnimationEventMethod = nullptr;

		MonoMethod* myOnRenderUIMethod = nullptr;

		MonoMethod* myOnTriggerEnterMethod = nullptr;
		MonoMethod* myOnTriggerExitMethod = nullptr;
		MonoMethod* myOnCollisionEnterMethod = nullptr;
		MonoMethod* myOnCollisionExitMethod = nullptr;

		MonoMethod* myOnEnable = nullptr;
		MonoMethod* myOnDisable = nullptr;

		GCHandle myHandle = nullptr;

		std::vector<std::string> myFieldNames;
		inline static Buffer myFieldBuffer;
	};

	template<typename T>
	inline const T MonoScriptInstance::GetField(const std::string& name)
	{
		myFieldBuffer.Allocate(40); // Don't know why sizeof(T) doesn't work here...

		bool success = GetFieldInternal(name, myFieldBuffer.As<void>());
		if (!success)
		{
			return T();
		}

		return *myFieldBuffer.As<T>();
	}
}
