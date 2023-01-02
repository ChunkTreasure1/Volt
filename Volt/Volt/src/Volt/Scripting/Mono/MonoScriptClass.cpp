#include "vtpch.h"
#include "MonoScriptClass.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Volt
{
	MonoScriptClass::MonoScriptClass(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& className)
		: myClassName(className), myNamespace(classNamespace)
	{
		myMonoClass = mono_class_from_name(assemblyImage, classNamespace.c_str(), className.c_str());
		FindAndCacheFields();
	}

	MonoObject* MonoScriptClass::Instantiate()
	{
		return MonoScriptEngine::InstantiateClass(myMonoClass);
	}

	MonoMethod* MonoScriptClass::GetMethod(const std::string& name, int32_t paramCount)
	{
		if (myMethodCache.contains(name))
		{
			return myMethodCache.at(name);
		}

		MonoMethod* method = mono_class_get_method_from_name(myMonoClass, name.c_str(), paramCount);
		myMethodCache.emplace(name, method);

		return method;
	}

	MonoObject* MonoScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		MonoObject* obj = mono_runtime_invoke(method, instance, params, &exception);

		if (exception)
		{
			mono_print_unhandled_exception(exception);
		}

		return obj;
	}

	bool MonoScriptClass::IsSubclassOf(Ref<MonoScriptClass> parent)
	{
		return mono_class_is_subclass_of(myMonoClass, parent->myMonoClass, false);
	}

	void MonoScriptClass::FindAndCacheFields()
	{
		const int32_t fieldCount = mono_class_num_fields(myMonoClass);

		void* iterator = nullptr;
		while (MonoClassField* field = mono_class_get_fields(myMonoClass, &iterator))
		{
			const char* fieldName = mono_field_get_name(field);

			const uint32_t flags = mono_field_get_flags(field);
			if (flags & MONO_ASSEMBLY_PUBLIC_KEY)
			{
				MonoType* fieldType = mono_field_get_type(field);
				const char* typeName = mono_type_get_name(fieldType);

				MonoScriptField scriptField{};
				scriptField.type = GetTypeFromString(typeName);
				scriptField.fieldPtr = field;

				myFields.emplace(fieldName, scriptField);
			}
		}
	}

	Wire::ComponentRegistry::PropertyType MonoScriptClass::GetTypeFromString(const std::string& str)
	{
		if (str == "System.Boolean")
		{
			return Wire::ComponentRegistry::PropertyType::Bool;
		}
		else if (str == "System.Int32")
		{
			return Wire::ComponentRegistry::PropertyType::Int;
		}
		else if (str == "System.UInt32")
		{
			return Wire::ComponentRegistry::PropertyType::UInt;
		}
		else if (str == "System.Int16")
		{
			return Wire::ComponentRegistry::PropertyType::Short;
		}
		else if (str == "System.Char")
		{
			return Wire::ComponentRegistry::PropertyType::Char;
		}
		else if (str == "System.Byte")
		{
			return Wire::ComponentRegistry::PropertyType::UChar;
		}
		else if (str == "System.Single")
		{
			return Wire::ComponentRegistry::PropertyType::Float;
		}
		else if (str == "System.Double")
		{
			return Wire::ComponentRegistry::PropertyType::Double;
		}
		else if (str == "Volt.Vector2")
		{
			return Wire::ComponentRegistry::PropertyType::Vector2;
		}
		else if (str == "Volt.Vector3")
		{
			return Wire::ComponentRegistry::PropertyType::Vector3;
		}
		else if (str == "Volt.Vector4")
		{
			return Wire::ComponentRegistry::PropertyType::Vector4;
		}
		else if (str == "Volt.Quaternion")
		{
			return Wire::ComponentRegistry::PropertyType::Quaternion;
		}
		else if (str == "System.String")
		{
			return Wire::ComponentRegistry::PropertyType::String;
		}
		else if (str == "System.Int64")
		{
			return Wire::ComponentRegistry::PropertyType::Int64;
		}
		else if (str == "System.UInt64")
		{
			return Wire::ComponentRegistry::PropertyType::UInt64;
		}
		else if (str == "Volt.Entity")
		{
			return Wire::ComponentRegistry::PropertyType::EntityId;
		}

		return Wire::ComponentRegistry::PropertyType::Float;
	}
}