#include "vtpch.h"
#include "MonoScriptEngine.h"

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"
#include "Volt/Core/Buffer.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Scripting/Mono/MonoScriptGlue.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"

#include <Wire/Entity.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Volt
{
	struct ScriptEngineData
	{
		MonoDomain* rootDomain = nullptr;

		MonoScriptEngine::AssemblyData coreData;
		MonoScriptEngine::AssemblyData appData;

		Scene* sceneContext = nullptr;

		Ref<MonoScriptClass> entityScriptClass;

		std::filesystem::path coreAssemblyPath;
		std::filesystem::path appAssemblyPath;

		std::unordered_map<std::string, Ref<MonoScriptClass>> entityClasses;
		std::unordered_map<Wire::EntityId, Ref<MonoScriptInstance>> entityInstances;
		std::unordered_map<Wire::EntityId, MonoScriptFieldMap> entityScriptFields;
	};

	static Scope<ScriptEngineData> s_monoData;

	namespace Utility
	{
		inline static MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& assemblyPath)
		{
			uint32_t fileSize = 0;
			Buffer buffer = Buffer::ReadFromFile(assemblyPath);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(buffer.As<char>(), (uint32_t)buffer.GetSize(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				VT_CORE_ERROR("[MonoScriptEngine] Failed to load C# assembly {0}", errorMessage);
				return nullptr;
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
			buffer.Release();

			return assembly;
		}

		inline static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* namespaceStr = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				VT_CORE_INFO("[MonoScriptEngine] Type found {0}.{1}!", namespaceStr, name);
			}
		}
	}

	void MonoScriptEngine::Initialize()
	{
		s_monoData = CreateScope<ScriptEngineData>();

		InitializeMono();
		s_monoData->coreData = LoadAssembly("Scripts/Volt-ScriptCore.dll", "VoltScriptRuntime", true);
		s_monoData->appData = LoadAssembly(Volt::ProjectManager::GetMonoAssemblyPath(), "ProjectTemplateRuntime", false);

		MonoScriptGlue::RegisterFunctions();

		s_monoData->entityScriptClass = CreateRef<MonoScriptClass>(s_monoData->coreData.assemblyImage, "Volt", "Entity");
		s_monoData->entityScriptClass->GetMethod(".ctor", 1);

		LoadAndCreateMonoClasses(s_monoData->appData.assembly);
	}

	void MonoScriptEngine::LoadAndCreateMonoClasses(MonoAssembly* assembly)
	{
		s_monoData->entityClasses.clear();

		MonoImage* image = mono_assembly_get_image(assembly);

		MonoClass* entityClass = mono_class_from_name(s_monoData->coreData.assemblyImage, "Volt", "Entity");

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* namespaceStr = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			MonoClass* monoClass = mono_class_from_name(s_monoData->appData.assemblyImage, namespaceStr, name);
			if (monoClass == entityClass)
			{
				continue;
			}

			const std::string typeName = std::string(namespaceStr) + "." + name;
			Ref<MonoScriptClass> scriptClass = CreateRef<MonoScriptClass>(image, namespaceStr, name);

			if (scriptClass->IsSubclassOf(s_monoData->entityScriptClass))
			{
				s_monoData->entityClasses.emplace(typeName, scriptClass);
			}
		}
	}

	void MonoScriptEngine::Shutdown()
	{
		ShutdownMono();
		s_monoData = nullptr;
	}

	void MonoScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_monoData->sceneContext = scene;
	}

	void MonoScriptEngine::OnRuntimeEnd()
	{
		s_monoData->entityInstances.clear();
		s_monoData->sceneContext = nullptr;
	}

	bool MonoScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_monoData->entityClasses.contains(fullClassName);
	}

	void MonoScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_monoData->coreData.domain);
	
		s_monoData->coreData = LoadAssembly(s_monoData->coreAssemblyPath, "VoltScriptRuntime", true);
		s_monoData->appData = LoadAssembly(s_monoData->appAssemblyPath, "ProjectTemplateRuntime", false);

		s_monoData->entityScriptClass = CreateRef<MonoScriptClass>(s_monoData->coreData.assemblyImage, "Volt", "Entity");
		s_monoData->entityScriptClass->GetMethod(".ctor", 1);

		LoadAndCreateMonoClasses(s_monoData->appData.assembly);
	
		VT_CORE_INFO("[MonoScriptEngine] C# Assembly has been reloaded!");
	}

	void MonoScriptEngine::OnCreateEntityInstance(Wire::EntityId entityId, const std::string& fullClassName)
	{
		if (EntityClassExists(fullClassName))
		{
			auto instance = CreateRef<MonoScriptInstance>(s_monoData->entityClasses.at(fullClassName), entityId);
			s_monoData->entityInstances[entityId] = instance;

			// Copy values
			VT_CORE_ASSERT(s_monoData->entityScriptFields.contains(entityId), "Entity ID does not exist in script fields!");
			const auto& fieldMap = s_monoData->entityScriptFields.at(entityId);
			for (const auto& [name, fieldInstance] : fieldMap)
			{
				instance->SetField(name, fieldInstance.data);
			}

			s_monoData->entityInstances.at(entityId)->InvokeOnCreate();
		}
	}

	void MonoScriptEngine::OnUpdateEntityInstance(Wire::EntityId entityId, float deltaTime)
	{
		VT_CORE_ASSERT(s_monoData->entityInstances.contains(entityId), "Entity does not have a script instance!");
		s_monoData->entityInstances.at(entityId)->InvokeOnUpdate(deltaTime);
	}

	void MonoScriptEngine::OnDestroyEntityInstance(Wire::EntityId entityId)
	{
		VT_CORE_ASSERT(s_monoData->entityInstances.contains(entityId), "Entity does not have a script instance!");
		s_monoData->entityInstances.at(entityId)->InvokeOnDestroy();
		s_monoData->entityInstances.erase(entityId);
	}

	Scene* MonoScriptEngine::GetSceneContext()
	{
		return s_monoData->sceneContext;
	}

	MonoDomain* MonoScriptEngine::GetAppDomain()
	{
		return s_monoData->coreData.domain;
	}

	Ref<MonoScriptClass> MonoScriptEngine::GetEntityMonoClass()
	{
		return s_monoData->entityScriptClass;
	}

	const Ref<MonoScriptClass> MonoScriptEngine::GetScriptClass(const std::string& name)
	{
		VT_CORE_ASSERT(s_monoData->entityClasses.contains(name), "Class does not exist!");
		return s_monoData->entityClasses.at(name);
	}

	Ref<MonoScriptInstance> MonoScriptEngine::GetInstanceFromEntityId(Wire::EntityId entityId)
	{
		if (!s_monoData->entityInstances.contains(entityId))
		{
			return nullptr;
		}

		return s_monoData->entityInstances.at(entityId);
	}

	MonoScriptFieldMap& MonoScriptEngine::GetScriptFieldMap(Wire::EntityId entityId)
	{
		VT_CORE_ASSERT(entityId != Wire::NullID, "Entity ID is not valid!");

		return s_monoData->entityScriptFields[entityId];
	}

	MonoScriptEngine::AssemblyData MonoScriptEngine::LoadAssembly(const std::filesystem::path& assemblyPath, const std::string& name, bool createDomain)
	{
		AssemblyData data{};

		if (createDomain)
		{
			std::string tempName = name;
			data.domain = mono_domain_create_appdomain(tempName.data(), nullptr);
			mono_domain_set(data.domain, true);
	
			s_monoData->coreAssemblyPath = assemblyPath;
		}
		else
		{
			s_monoData->appAssemblyPath = assemblyPath;
		}

		data.assembly = Utility::LoadCSharpAssembly(assemblyPath);
		data.assemblyImage = mono_assembly_get_image(data.assembly);

		return data;
	}

	void MonoScriptEngine::InitializeMono()
	{
		mono_set_assemblies_path("Scripts/mono/lib");

		MonoDomain* rootDomain = mono_jit_init("VoltJITRuntime");
		VT_CORE_ASSERT(rootDomain, "Root domain not initialized!");

		s_monoData->rootDomain = rootDomain;
	}

	void MonoScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_monoData->coreData.domain);
		mono_jit_cleanup(s_monoData->rootDomain);

		s_monoData->coreData.assembly = nullptr;
		s_monoData->coreData.domain = nullptr;
		s_monoData->rootDomain = nullptr;
	}

	MonoMethod* MonoScriptEngine::GetEntityConstructor()
	{
		return s_monoData->entityScriptClass->GetMethod(".ctor", 1);
	}

	MonoObject* MonoScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_monoData->coreData.domain, monoClass);
		mono_runtime_object_init(instance);

		return instance;
	}
}