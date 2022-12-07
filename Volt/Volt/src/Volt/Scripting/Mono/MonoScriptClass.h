#pragma once

#include "Volt/Core/Base.h"

#include <Wire/Wire.h>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
}

namespace Volt
{
	struct MonoScriptField
	{
		Wire::ComponentRegistry::PropertyType type;
		MonoClassField* fieldPtr = nullptr;
	};

	struct MonoScriptFieldInstance
	{
		MonoScriptField field;
		uint8_t data[16];

		template<typename T>
		void SetValue(const T& value)
		{
			(*(T*)&data[0]) = value;
		}
	};

	class MonoScriptClass
	{
	public:
		MonoScriptClass() = default;
		MonoScriptClass(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& className);

		MonoObject* Instantiate();
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
		MonoMethod* GetMethod(const std::string& name, int32_t paramCount);

		inline MonoClass* GetClass() const { return myMonoClass; }
		inline const std::unordered_map<std::string, MonoScriptField>& GetFields() const { return myFields; }

		bool IsSubclassOf(Ref<MonoScriptClass> parent);

	private:
		void FindAndCacheFields();
		Wire::ComponentRegistry::PropertyType GetTypeFromString(const std::string& str);

		std::string myNamespace;
		std::string myClassName;

		MonoClass* myMonoClass = nullptr;
		std::unordered_map<std::string, MonoMethod*> myMethodCache;
		std::unordered_map<std::string, MonoScriptField> myFields;
	};
}