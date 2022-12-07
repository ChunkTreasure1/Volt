#pragma once

#include "Volt/Core/Base.h"
#include <Wire/Wire.h>

extern "C"
{
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Volt
{
	class MonoScriptClass;
	class MonoScriptInstance
	{
	public:
		MonoScriptInstance(Ref<MonoScriptClass> monoClass, Wire::EntityId entityId);

		void InvokeOnCreate();
		void InvokeOnDestroy();
		void InvokeOnUpdate(float deltaTime);

		void SetField(const std::string& name, const void* value);

		template<typename T>
		const T GetField(const std::string& name);

		inline const Ref<MonoScriptClass> GetClass() const { return myMonoClass; }

	private:
		bool GetFieldInternal(const std::string& name, void* outData);
		bool SetFieldInternal(const std::string& name, const void* value);
		
		Ref<MonoScriptClass> myMonoClass;

		MonoMethod* myIdConstructorMethod = nullptr;
		MonoMethod* myCreateMethod = nullptr;
		MonoMethod* myDestroyMethod = nullptr;
		MonoMethod* myUpdateMethod = nullptr;

		MonoObject* myInstance;

		std::vector<std::string> myFieldNames;
		inline static uint8_t myFieldBuffer[16];
	};

	template<typename T>
	inline const T MonoScriptInstance::GetField(const std::string& name)
	{
		bool success = GetFieldInternal(name, myFieldBuffer);
		if (!success)
		{
			return T();
		}

		return *(T*)myFieldBuffer;
	}
}