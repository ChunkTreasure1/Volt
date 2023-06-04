#include "vtpch.h"
#include "MonoEnum.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Volt
{
	MonoEnum::MonoEnum(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& enumName)
		: myNamespace(classNamespace), myEnumName(enumName)
	{
		myMonoClass = mono_class_from_name(assemblyImage, classNamespace.c_str(), enumName.c_str());

		LoadEnumValues();
	}

	void MonoEnum::LoadEnumValues()
	{
		void* iterator = nullptr;
		uint32_t count = 0;

		MonoObject* tempObject = mono_object_new(MonoScriptEngine::GetAppDomain(), myMonoClass);
		while (MonoClassField* field = mono_class_get_fields(myMonoClass, &iterator))
		{
			if (count == 0)
			{
				count++;
				continue;
			}

			const char* fieldName = mono_field_get_name(field);

			MonoType* fieldType = mono_field_get_type(field);
			if (mono_type_get_type(fieldType) != MONO_TYPE_VALUETYPE)
			{
				uint32_t value;
				mono_field_get_value(tempObject, field, &value);

				myEnumValues.emplace_back(fieldName, value);
				continue;
			}

			MonoObject* valueObject = mono_field_get_value_object(MonoScriptEngine::GetAppDomain(), field, tempObject);
			uint32_t value = *(uint32_t*)mono_object_unbox(valueObject);
			myEnumValues.emplace_back(fieldName, value);
		}
	}
}
