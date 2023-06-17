#include "vtpch.h"
#include "MonoScriptClass.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Volt
{
	MonoScriptClass::MonoScriptClass(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& className, bool isEngineScript)
		: myClassName(className), myNamespace(classNamespace), myIsEngineScript(isEngineScript)
	{
		myMonoClass = mono_class_from_name(assemblyImage, classNamespace.c_str(), className.c_str());
		FindAndCacheFields();
	}

	MonoMethod* MonoScriptClass::GetMethod(const std::string& name, int32_t paramCount)
	{
		if (myMethodCache.contains(name))
		{
			return myMethodCache.at(name);
		}

		MonoMethod* method = mono_class_get_method_from_name(myMonoClass, name.c_str(), paramCount);

		if (!method)
		{
			method = TryGetMethodOfParent(myMonoClass, name, paramCount);
		}

		myMethodCache.emplace(name, method);
		return method;
	}

	bool MonoScriptClass::IsSubclassOf(Ref<MonoScriptClass> parent)
	{
		return mono_class_is_subclass_of(myMonoClass, parent->myMonoClass, false);
	}
	VT_OPTIMIZE_OFF
		void MonoScriptClass::FindAndCacheFields()
	{
		const int32_t fieldCount = mono_class_num_fields(myMonoClass);

		void* iterator = nullptr;

		while (MonoClassField* field = mono_class_get_fields(myMonoClass, &iterator))
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
			scriptField.type = GetTypeFromString(typeName);
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

			if (scriptField.type == MonoFieldType::Enum)
			{
				for (const auto& [enumName, monoEnum] : MonoScriptEngine::GetRegisteredEnums())
				{
					if (typeName == enumName)
					{
						scriptField.enumName = enumName;
						break;
					}
				}
			}
			myFields.emplace(fieldName, scriptField);
		}

		FindAndCacheSubClassFields(myMonoClass);
	}
	VT_OPTIMIZE_ON

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
			scriptField.type = GetTypeFromString(typeName);
			scriptField.fieldPtr = field;
			scriptField.fieldAccessability = accessibility;

			if (scriptField.type == MonoFieldType::Enum)
			{
				for (const auto& [enumName, monoEnum] : MonoScriptEngine::GetRegisteredEnums())
				{
					if (typeName == enumName)
					{
						scriptField.enumName = enumName;
						break;
					}
				}
			}

			myFields.emplace(fieldName, scriptField);
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

	MonoFieldType MonoScriptClass::GetTypeFromString(const std::string& str)
	{
		if (str == "System.Boolean")
		{
			return MonoFieldType::Bool;
		}
		else if (str == "System.Int32")
		{
			return MonoFieldType::Int;
		}
		else if (str == "System.UInt32")
		{
			return MonoFieldType::UInt;
		}
		else if (str == "System.Int16")
		{
			return MonoFieldType::Short;
		}
		else if (str == "System.Char")
		{
			return MonoFieldType::Char;
		}
		else if (str == "System.Byte")
		{
			return MonoFieldType::UChar;
		}
		else if (str == "System.Single")
		{
			return MonoFieldType::Float;
		}
		else if (str == "System.Double")
		{
			return MonoFieldType::Double;
		}
		else if (str == "Volt.Vector2")
		{
			return MonoFieldType::Vector2;
		}
		else if (str == "Volt.Vector3")
		{
			return MonoFieldType::Vector3;
		}
		else if (str == "Volt.Vector4")
		{
			return MonoFieldType::Vector4;
		}
		else if (str == "System.String")
		{
			return MonoFieldType::String;
		}
		else if (str == "System.Int64")
		{
			return MonoFieldType::Int64;
		}
		else if (str == "System.UInt64")
		{
			return MonoFieldType::UInt64;
		}
		else if (str == "Volt.Entity")
		{
			return MonoFieldType::Entity;
		}
		else if (str == "Volt.Asset")
		{
			return MonoFieldType::Asset;
		}
		else if (str == "Volt.Animation")
		{
			return MonoFieldType::Animation;
		}
		else if (str == "Volt.Prefab")
		{
			return MonoFieldType::Prefab;
		}
		else if (str == "Volt.Scene")
		{
			return MonoFieldType::Scene;
		}
		else if (str == "Volt.Mesh")
		{
			return MonoFieldType::Mesh;
		}
		else if (str == "Volt.Font")
		{
			return MonoFieldType::Font;
		}
		else if (str == "Volt.Material")
		{
			return MonoFieldType::Material;
		}
		else if (str == "Volt.Texture")
		{
			return MonoFieldType::Texture;
		}
		else if (str == "Volt.Color")
		{
			return MonoFieldType::Color;
		}
		else if (str == "Volt.PostProcessingMaterial")
		{
			return MonoFieldType::PostProcessingMaterial;
		}
		else if (str == "Volt.Video")
		{
			return MonoFieldType::Video;
		}
		else if (str == "Volt.AnimationGraph")
		{
			return MonoFieldType::AnimationGraph;
		}
		else if (str == "Volt.Quaternion")
		{
			return MonoFieldType::Quaternion;
		}

		// Check if it's an enum
		for (const auto& [typeName, monoEnum] : MonoScriptEngine::GetRegisteredEnums())
		{
			if (str == typeName)
			{
				return MonoFieldType::Enum;
			}
		}

		return MonoFieldType::Unknown;
	}

	MonoFieldType MonoScriptClass::WirePropTypeToMonoFieldType(const Wire::ComponentRegistry::PropertyType& type)
	{
		switch (type)
		{
			case Wire::ComponentRegistry::PropertyType::Bool: return MonoFieldType::Bool;
			case Wire::ComponentRegistry::PropertyType::Int: return MonoFieldType::Int;
			case Wire::ComponentRegistry::PropertyType::UInt: return MonoFieldType::UInt;
			case Wire::ComponentRegistry::PropertyType::Short: return MonoFieldType::Short;
			case Wire::ComponentRegistry::PropertyType::UShort: return MonoFieldType::UShort;
			case Wire::ComponentRegistry::PropertyType::Char: return MonoFieldType::Char;
			case Wire::ComponentRegistry::PropertyType::UChar: return MonoFieldType::UChar;
			case Wire::ComponentRegistry::PropertyType::Float: return MonoFieldType::Float;
			case Wire::ComponentRegistry::PropertyType::Double: return MonoFieldType::Double;
			case Wire::ComponentRegistry::PropertyType::Vector2: return MonoFieldType::Vector2;
			case Wire::ComponentRegistry::PropertyType::Vector3: return MonoFieldType::Vector3;
			case Wire::ComponentRegistry::PropertyType::Vector4: return MonoFieldType::Vector4;
			case Wire::ComponentRegistry::PropertyType::String: return MonoFieldType::String;
			case Wire::ComponentRegistry::PropertyType::Int64: return MonoFieldType::Int64;
			case Wire::ComponentRegistry::PropertyType::UInt64: return MonoFieldType::UInt64;

			case Wire::ComponentRegistry::PropertyType::Quaternion: return MonoFieldType::Quaternion;
			case Wire::ComponentRegistry::PropertyType::Color4: return MonoFieldType::Color;

			case Wire::ComponentRegistry::PropertyType::AssetHandle: return MonoFieldType::Asset;
			case Wire::ComponentRegistry::PropertyType::EntityId: return MonoFieldType::Entity;
			case Wire::ComponentRegistry::PropertyType::Enum: return MonoFieldType::Enum;

			default: return MonoFieldType::Unknown;
		}
	}

	Wire::ComponentRegistry::PropertyType MonoScriptClass::MonoFieldTypeToWirePropType(const MonoFieldType& type)
	{
		if (IsAsset(type))
		{
			return Wire::ComponentRegistry::PropertyType::AssetHandle;
		}

		switch (type)
		{
			case MonoFieldType::Bool: return Wire::ComponentRegistry::PropertyType::Bool;
			case MonoFieldType::Int: return Wire::ComponentRegistry::PropertyType::Int;
			case MonoFieldType::UInt: return Wire::ComponentRegistry::PropertyType::UInt;
			case MonoFieldType::Short: return Wire::ComponentRegistry::PropertyType::Short;
			case MonoFieldType::UShort: return Wire::ComponentRegistry::PropertyType::UShort;
			case MonoFieldType::Char: return Wire::ComponentRegistry::PropertyType::Char;
			case MonoFieldType::UChar: return Wire::ComponentRegistry::PropertyType::UChar;
			case MonoFieldType::Float: return Wire::ComponentRegistry::PropertyType::Float;
			case MonoFieldType::Double: return Wire::ComponentRegistry::PropertyType::Double;
			case MonoFieldType::Vector2: return Wire::ComponentRegistry::PropertyType::Vector2;
			case MonoFieldType::Vector3: return Wire::ComponentRegistry::PropertyType::Vector3;
			case MonoFieldType::Vector4: return Wire::ComponentRegistry::PropertyType::Vector4;
			case MonoFieldType::String: return Wire::ComponentRegistry::PropertyType::String;
			case MonoFieldType::Int64: return Wire::ComponentRegistry::PropertyType::Int64;
			case MonoFieldType::UInt64: return Wire::ComponentRegistry::PropertyType::UInt64;

			case MonoFieldType::Quaternion: return Wire::ComponentRegistry::PropertyType::Quaternion;
			case MonoFieldType::Color: return Wire::ComponentRegistry::PropertyType::Color4;

			case MonoFieldType::Entity: return Wire::ComponentRegistry::PropertyType::EntityId;
			case MonoFieldType::Enum: return Wire::ComponentRegistry::PropertyType::Enum;

			default: return Wire::ComponentRegistry::PropertyType::Unknown;
		}
	}

	bool MonoScriptClass::IsAsset(const MonoFieldType& type)
	{
		switch (type)
		{
			case MonoFieldType::Scene:
			case MonoFieldType::Animation:
			case MonoFieldType::Prefab:
			case MonoFieldType::Mesh:
			case MonoFieldType::Font:
			case MonoFieldType::Material:
			case MonoFieldType::Texture:
			case MonoFieldType::PostProcessingMaterial:
			case MonoFieldType::Video:
			case MonoFieldType::AnimationGraph:
			case MonoFieldType::Asset:
			{
				return true;
			};

			default:
			{
				return false;
			}
		}
	}
}
