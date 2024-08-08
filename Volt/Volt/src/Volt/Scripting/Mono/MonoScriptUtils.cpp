#include "vtpch.h"
#include "MonoScriptUtils.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptEntity.h"
#include "Volt/Scripting/Mono/MonoGCManager.h"

#include "Volt/Utility/PremadeCommands.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>

namespace Volt
{
	std::string MonoScriptUtils::GetClassName2(const std::string& fullClassName)
	{
		size_t lastDotPos = fullClassName.find_last_of(".");
		if (lastDotPos != std::string::npos)
		{
			return fullClassName.substr(lastDotPos + 1);
		}
		else
		{
			return fullClassName;
		}
	}

	std::string MonoScriptUtils::GetNameSpace(const std::string& fullClassName)
	{
		size_t lastDotPos = fullClassName.find_last_of(".");
		if (lastDotPos != std::string::npos)
		{
			return fullClassName.substr(0, lastDotPos);
		}
		else
		{
			return "";
		}
	}

	void MonoScriptUtils::RegisterArrayTypes()
	{
		VT_REGISTER_MONO_ARRAY_TYPE(bool, "System.Boolean");
		VT_REGISTER_MONO_ARRAY_TYPE(char, "System.Char");
		VT_REGISTER_MONO_ARRAY_TYPE(float, "System.Single");
		VT_REGISTER_MONO_ARRAY_TYPE(double, "System.Double");
		VT_REGISTER_MONO_ARRAY_TYPE(int8_t, "System.SByte");
		VT_REGISTER_MONO_ARRAY_TYPE(uint8_t, "System.Byte");
		VT_REGISTER_MONO_ARRAY_TYPE(int16_t, "System.Int16");
		VT_REGISTER_MONO_ARRAY_TYPE(uint16_t, "System.UInt16");
		VT_REGISTER_MONO_ARRAY_TYPE(int32_t, "System.Int32");
		VT_REGISTER_MONO_ARRAY_TYPE(uint32_t, "System.UInt32");
		VT_REGISTER_MONO_ARRAY_TYPE(int64_t, "System.Int64");
		VT_REGISTER_MONO_ARRAY_TYPE(uint64_t, "System.UInt64");
	}

	const std::string MonoScriptUtils::GetStringFromMonoString(MonoString* string)
	{
		char* cStr = mono_string_to_utf8(string);

		if (!cStr)
		{
			return {};
		}

		std::string str(cStr);
		mono_free(cStr);

		return str;
	}

	MonoString* MonoScriptUtils::GetMonoStringFromString(const std::string& string)
	{
		return mono_string_new(MonoScriptEngine::GetAppDomain(), string.c_str());
	}

	FieldAccessibility MonoScriptUtils::GetFieldAccessabilityLevel(MonoClassField* field)
	{
		FieldAccessibility accessibility = FieldAccessibility::None;
		uint32_t accessFlag = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

		switch (accessFlag)
		{
			case MONO_FIELD_ATTR_PRIVATE:
			{
				accessibility = FieldAccessibility::Private;
				break;
			}

			case MONO_FIELD_ATTR_FAM_AND_ASSEM:
			{
				accessibility = FieldAccessibility::Protected | FieldAccessibility::Internal;
				break;
			}

			case MONO_FIELD_ATTR_ASSEMBLY:
			{
				accessibility = FieldAccessibility::Internal;
				break;
			}

			case MONO_FIELD_ATTR_FAMILY:
			{
				accessibility = FieldAccessibility::Protected;
				break;
			}

			case MONO_FIELD_ATTR_FAM_OR_ASSEM:
			{
				accessibility = FieldAccessibility::Private | FieldAccessibility::Protected;
				break;
			}

			case MONO_FIELD_ATTR_PUBLIC:
			{
				accessibility = FieldAccessibility::Public;
				break;
			}
		}

		return accessibility;
	}

	// THIS IS NOT WORKING ATM
// -----

	MonoArray* MonoScriptUtils::InternalCreateMonoArray(const std::string& aNamespace, const std::string& aClass, const Vector<void*> data)
	{
		auto& coreAssembly = MonoScriptEngine::GetCoreAssembly();
		MonoClass* elementClass = mono_class_from_name(coreAssembly.assemblyImage, aNamespace.c_str(), aClass.c_str());
		if (elementClass == nullptr)
		{
			// Could not find the class in the specified namespace
			return nullptr;
		}
		uintptr_t length = 0;
		MonoArray* array = mono_array_new_full(coreAssembly.domain, elementClass, &length, nullptr);

		for (int i = 0; i < data.size(); i++)
		{
			// This needs to get fixed
			//mono_array_set(array, elementClass, i, data[i]);
		}

		return array;
	}

	// -----

	MonoArray* MonoScriptUtils::CreateMonoArrayUInt32(const Vector<uint32_t>& vector)
	{
		auto& coreAssembly = MonoScriptEngine::GetCoreAssembly();
		MonoArray* array = mono_array_new(coreAssembly.domain, mono_get_uint32_class(), vector.size());
		for (uint16_t i = 0; i < vector.size(); ++i)
		{
			mono_array_set(array, uint32_t, i, vector[i]);
		}
		return array;
	}

	MonoArray* MonoScriptUtils::CreateMonoArrayUInt64(const Vector<uint64_t>& vector)
	{
		auto& coreAssembly = MonoScriptEngine::GetCoreAssembly();
		MonoArray* array = mono_array_new(coreAssembly.domain, mono_get_uint64_class(), vector.size());
		for (uint16_t i = 0; i < vector.size(); ++i)
		{
			mono_array_set(array, uint64_t, i, vector[i]);
		}
		return array;
	}

	MonoArray* MonoScriptUtils::CreateMonoArrayEntity(const Vector<EntityID>& vector)
	{
		auto& coreAssembly = MonoScriptEngine::GetCoreAssembly();
		Vector<MonoObject*> monoVector;

		for (uint16_t i = 0; i < vector.size(); ++i)
		{
			auto monoEnt = MonoScriptEngine::GetEntityFromId(vector[i]);
			if (!monoEnt)
			{
				monoEnt = MonoScriptEngine::GetOrCreateMonoEntity(vector[i]);
			}
			monoVector.emplace_back(MonoGCManager::GetObjectFromHandle(monoEnt->GetHandle()));
		}

		MonoArray* array = mono_array_new(coreAssembly.domain, MonoScriptEngine::GetEntityClass()->GetClass(), vector.size());
		for (uint16_t i = 0; i < monoVector.size(); ++i)
		{
			mono_array_set(array, MonoObject*, i, monoVector[i]);
		}
		return array;
	}

	MonoArray* MonoScriptUtils::CreateMonoArrayEntity(const Vector<Entity>& vector)
	{
		auto& coreAssembly = MonoScriptEngine::GetCoreAssembly();
		Vector<MonoObject*> monoVector;

		for (uint16_t i = 0; i < vector.size(); ++i)
		{
			auto monoEnt = MonoScriptEngine::GetEntityFromId(vector[i].GetID());
			if (!monoEnt)
			{
				monoEnt = MonoScriptEngine::GetOrCreateMonoEntity(vector[i].GetID());
			}
			monoVector.emplace_back(MonoGCManager::GetObjectFromHandle(monoEnt->GetHandle()));
		}

		MonoArray* array = mono_array_new(coreAssembly.domain, MonoScriptEngine::GetEntityClass()->GetClass(), vector.size());
		for (uint16_t i = 0; i < monoVector.size(); ++i)
		{
			mono_array_set(array, MonoObject*, i, monoVector[i]);
		}
		return array;
	}

	bool MonoScriptUtils::CreateNewCSFile(std::string name, std::filesystem::path directoryFromAssets, bool regenerate)
	{
		auto parentDir = Volt::ProjectManager::GetDirectory() / ((directoryFromAssets.empty()) ? "Assets/Scripts" : directoryFromAssets.string());
		auto newFilePath = Volt::ProjectManager::GetDirectory() / parentDir / (name + ".cs");
		
		if (!FileSystem::Exists(parentDir)) { std::filesystem::create_directory(newFilePath.parent_path()); }
		if (!FileSystem::Exists(newFilePath))
		{
			auto templateFilePath = Volt::ProjectManager::GetEngineDirectory() / "Volt-ScriptCore/Source/Volt/ProjectTemplate.cs";

			std::fstream templateFile(templateFilePath, std::ios::in);
			std::fstream newFile(newFilePath, std::ios::out);

			if (!templateFile.is_open())
			{
				VT_LOG(Warning, std::format("Could not open file: {0}", templateFilePath.filename().string()));
			}

			if (!newFile.is_open())
			{
				VT_LOG(Warning, std::format("Could not open file: {0}", newFilePath.filename().string()));
			}

			// Read the entire content of the template file into a string
			std::string content((std::istreambuf_iterator<char>(templateFile)),
				std::istreambuf_iterator<char>());

			// Replace all instances of "test" with the desired string
			size_t pos = 0;
			std::string findString = "ProjectTemplate";
			while ((pos = content.find(findString, pos)) != std::string::npos)
			{
				content.replace(pos, findString.length(), name);
				pos += name.length();
			}

			// Write the modified content to the new file
			newFile << content;

			templateFile.close();
			newFile.close();

			if (regenerate)
			{
				Volt::PremadeCommands::RunProjectPremakeCommand();
			}
		}

		return true;
	}
}
