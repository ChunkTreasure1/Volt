#include "vtpch.h"
#include "MonoScriptClass.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Volt
{
	MonoScriptClass::MonoScriptClass(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& className, bool isEngineScript)
		: m_className(className), m_namespace(classNamespace), m_isEngineScript(isEngineScript)
	{
		m_monoClass = mono_class_from_name(assemblyImage, classNamespace.c_str(), className.c_str());
		FindAndCacheFields();
	}

	MonoMethod* MonoScriptClass::GetMethod(const std::string& name, int32_t paramCount)
	{
		if (m_methodCache.contains(name))
		{
			return m_methodCache.at(name);
		}

		MonoMethod* method = mono_class_get_method_from_name(m_monoClass, name.c_str(), paramCount);

		if (!method)
		{
			method = TryGetMethodOfParent(m_monoClass, name, paramCount);
		}

		m_methodCache.emplace(name, method);
		return method;
	}

	bool MonoScriptClass::IsSubclassOf(Ref<MonoScriptClass> parent)
	{
		return mono_class_is_subclass_of(m_monoClass, parent->m_monoClass, false);
	}

	void MonoScriptClass::FindAndCacheFields()
	{
		void* iterator = nullptr;
		while (MonoClassField* field = mono_class_get_fields(m_monoClass, &iterator))
		{
			const auto accessibility = MonoScriptUtils::GetFieldAccessabilityLevel(field);

			MonoCustomAttrInfo* attributeInfo = mono_custom_attrs_from_field(GetClass(), field);

			if (attributeInfo != nullptr)
			{
				MonoClass* hideInEditorAttrClass = mono_class_from_name(MonoScriptEngine::GetCoreAssembly().assemblyImage, "Volt", "HideInEditorAttribute");
				MonoObject* attributeObj = mono_custom_attrs_get_attr(attributeInfo, hideInEditorAttrClass);
				if (attributeObj != nullptr)
				{
					// Has attribute
					continue;
				}
			}

			if (accessibility != FieldAccessibility::Public)
			{
				continue;
			}

			const char* fieldName = mono_field_get_name(field);
			MonoType* fieldType = mono_field_get_type(field);
			const char* typeName = mono_type_get_name(fieldType);

			MonoScriptField scriptField{};
			scriptField.type = MonoTypeRegistry::GetTypeInfo(typeName);
			scriptField.fieldPtr = field;
			scriptField.fieldAccessability = accessibility;

			/*if (attributeInfo != nullptr)
			{
				MonoClass* hideInEditorAttrClass = mono_class_from_name(MonoScriptEngine::GetCoreAssembly().assemblyImage, "Volt", "RepContinuous");
				MonoObject* attributeObj = mono_custom_attrs_get_attr(attributeInfo, hideInEditorAttrClass);
				if (attributeObj != nullptr)
				{
					scriptField.netData.replicatedCondition = eRepCondition::CONTINUOUS;
				}
			}*/

			if (attributeInfo != nullptr)
			{
				if (MonoScriptEngine::NetFieldSetup(this, "RepNotify", scriptField))
				{
					scriptField.netData.replicatedCondition = eRepCondition::NOTIFY;
				}
				else if (MonoScriptEngine::NetFieldSetup(this, "RepContinuous", scriptField))
				{
					scriptField.netData.replicatedCondition = eRepCondition::CONTINUOUS;
				}
			}

			// #TODO_Ivar: Reimplement
			//if (scriptField.type == MonoFieldType::Enum)
			//{
			//	for (const auto& [enumName, monoEnum] : MonoScriptEngine::GetRegisteredEnums())
			//	{
			//		if (typeName == enumName)
			//		{
			//			scriptField.enumName = enumName;
			//			break;
			//		}
			//	}
			//}
			m_fields.emplace(fieldName, scriptField);
		}

		FindAndCacheSubClassFields(m_monoClass);
	}

	void MonoScriptClass::FindAndCacheSubClassFields(MonoClass* klass)
	{
		auto parent = mono_class_get_parent(klass);
		if (!parent) { return; }

		std::string klassParentName = mono_class_get_name(parent);
		if (klassParentName == MonoScriptEngine::CORE_CLASS_NAME) { return; }

		void* iterator = nullptr;

		while (MonoClassField* field = mono_class_get_fields(parent, &iterator))
		{
			const auto accessibility = MonoScriptUtils::GetFieldAccessabilityLevel(field);

			if (accessibility != FieldAccessibility::Public)
			{
				continue;
			}

			const char* fieldName = mono_field_get_name(field);
			MonoType* fieldType = mono_field_get_type(field);
			const char* typeName = mono_type_get_name(fieldType);

			MonoScriptField scriptField{};
			scriptField.type = MonoTypeRegistry::GetTypeInfo(typeName);
			scriptField.fieldPtr = field;
			scriptField.fieldAccessability = accessibility;

			// #TODO_Ivar: Reimplement
			//if (scriptField.type == MonoFieldType::Enum)
			//{
			//	for (const auto& [enumName, monoEnum] : MonoScriptEngine::GetRegisteredEnums())
			//	{
			//		if (typeName == enumName)
			//		{
			//			scriptField.enumName = enumName;
			//			break;
			//		}
			//	}
			//}

			m_fields.emplace(fieldName, scriptField);
		}

		FindAndCacheSubClassFields(parent);
	}

	MonoMethod* MonoScriptClass::TryGetMethodOfParent(MonoClass* klass, const std::string& methodName, int32_t paramCount)
	{
		auto parent = mono_class_get_parent(klass);
		if (!parent) { return nullptr; }

		std::string klassParentName = mono_class_get_name(parent);
		if (klassParentName == MonoScriptEngine::CORE_CLASS_NAME) { return nullptr; }

		MonoMethod* method = mono_class_get_method_from_name(parent, methodName.c_str(), paramCount);

		if (method)
		{
			return method;
		}

		return TryGetMethodOfParent(parent, methodName, paramCount);
	}
}
