#include "vtpch.h"
#include "MonoScriptEngine.h"

#include "Volt/Core/Base.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Core/Buffer.h"
#include "Volt/Core/Application.h"

#include "Volt/Log/Log.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Scripting/Mono/MonoScriptGlue.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Scripting/Mono/MonoScriptEntity.h"
#include "Volt/Scripting/Mono/MonoCoreInstance.h"
#include "Volt/Scripting/Mono/MonoGCManager.h"
#include "Volt/Scripting/Mono/MonoEnum.h"
#include "Volt/Scripting/Mono/MonoTypeRegistry.h"

#include "Volt/Scene/SceneManager.h"
#include "Volt/Components/CoreComponents.h"

#include "Volt/Utility/StringUtility.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-gc.h>
#include <mono/utils/mono-logger.h>

namespace Volt
{
	struct ScriptEngineData
	{
		MonoDomain* rootDomain = nullptr;

		MonoScriptEngine::AssemblyData coreData;
		MonoScriptEngine::AssemblyData appData;

		Scene* sceneContext = nullptr;

		Ref<MonoScriptClass> coreScriptClass;
		Ref<MonoScriptClass> coreEntityClass;

		std::filesystem::path coreAssemblyPath;
		std::filesystem::path appAssemblyPath;

		std::unordered_map<std::string, MonoAssembly*> referencedAssemblies;
		std::unordered_map<std::string, Ref<MonoScriptClass>> scriptClasses;
		std::unordered_map<std::string, Ref<MonoScriptClass>> coreClasses;
		std::unordered_map<std::string, Ref<MonoEnum>> monoEnums;

		std::map<UUID, Ref<MonoScriptInstance>> scriptInstances;
		std::map<UUID, Ref<MonoCoreInstance>> coreInstances;

		std::unordered_map<entt::entity, Ref<MonoScriptEntity>> scriptEntities;
		std::unordered_map<std::string, MonoScriptFieldMap> scriptFieldsDefault;

		std::vector<UUID> scriptOnCreateQueue;
		std::vector<UUID> scriptDestroyQueue;

		bool enableDebugging = false;
		std::mutex mutex;
	};

	static Scope<ScriptEngineData> s_monoData;

	namespace Utility
	{
		inline static MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			Buffer buffer = Buffer::ReadFromFile(assemblyPath);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(buffer.As<char>(), (uint32_t)buffer.GetSize(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				VT_CORE_ERROR("[MonoScriptEngine] Failed to load C# assembly {0}", errorMessage);
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					Buffer pdbBuffer = Buffer::ReadFromFile(pdbPath);
					mono_debug_open_image_from_memory(image, pdbBuffer.As<const mono_byte>(), (int32_t)pdbBuffer.GetSize());
					VT_CORE_INFO("Loaded PDB {}", pdbPath.string());
					pdbBuffer.Release();
				}
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
			buffer.Release();
			mono_image_close(image);

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

		struct MonoExceptionInfo
		{
			std::string typeName;
			std::string source;
			std::string message;
			std::string stackTrace;
		};

		inline static MonoExceptionInfo GetExceptionInfo(MonoObject* exception)
		{
			MonoClass* exceptionClass = mono_object_get_class(exception);
			MonoType* exceptionType = mono_class_get_type(exceptionClass);

			auto GetExceptionString = [exception, exceptionClass](const char* stringName) -> std::string
			{
				MonoProperty* property = mono_class_get_property_from_name(exceptionClass, stringName);

				if (property == nullptr)
					return "";

				MonoMethod* getterMethod = mono_property_get_get_method(property);

				if (getterMethod == nullptr)
					return "";

				MonoString* string = (MonoString*)mono_runtime_invoke(getterMethod, exception, NULL, NULL);
				std::string funcString;

				if (string)
				{
					char* cStr = mono_string_to_utf8(string);
					funcString = cStr;
					mono_free(cStr);
				}

				return funcString;
			};

			return { mono_type_get_name(exceptionType), GetExceptionString("Source"), GetExceptionString("Message"), GetExceptionString("StackTrace") };
		}
	}

	void MonoScriptEngine::Initialize()
	{
		s_monoData = CreateScope<ScriptEngineData>();

		MonoTypeRegistry::Initialize();

		InitializeMono();
		bool status = LoadAssembly("Scripts/Volt-ScriptCore.dll");
		if (!status)
		{
			VT_CORE_ERROR("[MonoScriptEngine] Failed to load Volt-ScriptCore.dll");
			return;
		}

		const auto appPath = Volt::ProjectManager::GetMonoAssemblyPath();
		status = LoadAppAssembly(appPath);
		if (!status)
		{
			VT_CORE_ERROR("[MonoScriptEngine] Failed to load app assembly");
			return;
		}

		MonoScriptGlue::RegisterFunctions();

		s_monoData->coreScriptClass = CreateRef<MonoScriptClass>(s_monoData->coreData.assemblyImage, "Volt", CORE_CLASS_NAME.c_str());
		s_monoData->coreScriptClass->GetMethod(".ctor", 2);

		s_monoData->coreEntityClass = CreateRef<MonoScriptClass>(s_monoData->coreData.assemblyImage, "Volt", ENTITY_CLASS_NAME.c_str());
		s_monoData->coreEntityClass->GetMethod(".ctor", 2);

		LoadAndCreateCoreMonoClasses(s_monoData->coreData.assembly);
		LoadAndCreateMonoClasses(s_monoData->appData.assembly);
	}

	bool MonoScriptEngine::NetFieldSetup(MonoScriptClass* monoClass, const std::string& attibuteName, MonoScriptField& out_field)
	{
		MonoCustomAttrInfo* attributeInfo = mono_custom_attrs_from_field(monoClass->GetClass(), out_field.fieldPtr);
		if (!attributeInfo) return false;

		MonoClass* attributeClass = mono_class_from_name(MonoScriptEngine::GetCoreAssembly().assemblyImage, "Volt", attibuteName.c_str());
		if (!attributeClass) return false;
		MonoObject* attributeObj = mono_custom_attrs_get_attr(attributeInfo, attributeClass);
		if (!attributeObj) return false;

		if (auto notifyField = mono_class_get_field_from_name(attributeClass, "function"))
		{
			MonoType* fieldType = mono_field_get_type(notifyField);
			if (fieldType && mono_type_get_type(fieldType) == MONO_TYPE_STRING)
			{
				MonoString* monoData = *(MonoString**)((char*)attributeObj + mono_field_get_offset(notifyField));
				const char* data = mono_string_to_utf8(monoData);
				out_field.netData.boundFunction = data;
				return true;

				//Buffer buffer{};
				//buffer.Resize(128);
				//mono_field_get_value(attributeObj, notifyField, buffer.As<void>());
				//out_field.netData.boundFunction = std::string(buffer.As<const char>());
				//buffer.Release();
			}
		}
		return true;
	}

	bool MonoScriptEngine::LoadAssembly(const std::filesystem::path& assemblyPath)
	{
		std::string assemblyName = "VoltScriptRuntime";
		s_monoData->coreData.domain = mono_domain_create_appdomain(assemblyName.data(), nullptr);
		mono_domain_set(s_monoData->coreData.domain, true);
		mono_domain_set_config(s_monoData->coreData.domain, ".", "");

		s_monoData->coreAssemblyPath = assemblyPath;
		s_monoData->coreData.assembly = Utility::LoadCSharpAssembly(assemblyPath, s_monoData->enableDebugging);
		if (!s_monoData->coreData.assembly)
		{
			return false;
		}

		s_monoData->coreData.assemblyImage = mono_assembly_get_image(s_monoData->coreData.assembly);

		const auto referencedNames = GetReferencedAssembliesName(s_monoData->coreData.assemblyImage);
		LoadReferencedAssemblies(referencedNames);
		return true;
	}

	bool MonoScriptEngine::LoadAppAssembly(const std::filesystem::path& assemblyPath)
	{
		s_monoData->appAssemblyPath = assemblyPath;
		s_monoData->appData.assembly = Utility::LoadCSharpAssembly(assemblyPath, s_monoData->enableDebugging);
		if (!s_monoData->appData.assembly)
		{
			return false;
		}

		s_monoData->appData.assemblyImage = mono_assembly_get_image(s_monoData->appData.assembly);

		const auto referencedNames = GetReferencedAssembliesName(s_monoData->appData.assemblyImage);
		LoadReferencedAssemblies(referencedNames);
		return true;
	}

	void MonoScriptEngine::LoadAndCreateCoreMonoClasses(MonoAssembly* assembly)
	{
		s_monoData->coreClasses.clear();
		s_monoData->scriptClasses.clear();
		s_monoData->monoEnums.clear();

		MonoImage* image = mono_assembly_get_image(assembly);
		MonoClass* monoScriptClass = mono_class_from_name(s_monoData->coreData.assemblyImage, "Volt", CORE_CLASS_NAME.c_str());

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* namespaceStr = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			MonoClass* monoClass = mono_class_from_name(s_monoData->coreData.assemblyImage, namespaceStr, name);
			if (!monoClass)
			{
				continue;
			}

			if (monoClass == monoScriptClass)
			{
				continue;
			}

			if (mono_class_is_delegate(monoClass))
			{
				continue;
			}

			const std::string typeName = std::string(namespaceStr) + "." + name;
			if (mono_class_is_enum(monoClass))
			{
				Ref<MonoEnum> monoEnum = CreateRef<MonoEnum>(image, namespaceStr, name);
				s_monoData->monoEnums.emplace(typeName, monoEnum);
				continue;
			}

			// Check for engine script attribute
			{
				MonoCustomAttrInfo* attributeInfo = mono_custom_attrs_from_class(monoClass);
				if (!attributeInfo)
				{
					continue;
				}

				MonoClass* engineScriptAttrClass = mono_class_from_name(s_monoData->coreData.assemblyImage, "Volt", "EngineScript");
				if (!engineScriptAttrClass)
				{
					continue;
				}

				MonoObject* attributeObject = mono_custom_attrs_get_attr(attributeInfo, engineScriptAttrClass);
				if (!attributeObject)
				{
					continue;
				}
			}

			Ref<MonoScriptClass> scriptClass = CreateRef<MonoScriptClass>(image, namespaceStr, name, true);

			if (scriptClass->IsSubclassOf(s_monoData->coreScriptClass))
			{
				s_monoData->scriptClasses.emplace(typeName, scriptClass);
				SetScriptFieldDefaultData(1, entt::null, typeName);
			}
			else
			{
				s_monoData->coreClasses.emplace(typeName, scriptClass);
			}
		}
	}

	void MonoScriptEngine::LoadAndCreateMonoClasses(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		MonoClass* monoScriptClass = mono_class_from_name(s_monoData->coreData.assemblyImage, "Volt", CORE_CLASS_NAME.c_str());

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* namespaceStr = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			MonoClass* monoClass = mono_class_from_name(s_monoData->appData.assemblyImage, namespaceStr, name);
			if (!monoClass)
			{
				continue;
			}

			if (monoClass == monoScriptClass)
			{
				continue;
			}

			if (mono_class_is_delegate(monoClass))
			{
				continue;
			}

			const std::string typeName = std::string(namespaceStr) + "." + name;
			if (mono_class_is_enum(monoClass))
			{
				Ref<MonoEnum> monoEnum = CreateRef<MonoEnum>(image, namespaceStr, name);
				s_monoData->monoEnums.emplace(typeName, monoEnum);
				continue;
			}

			Ref<MonoScriptClass> scriptClass = CreateRef<MonoScriptClass>(image, namespaceStr, name);

			if (scriptClass->IsSubclassOf(s_monoData->coreScriptClass))
			{
				s_monoData->scriptClasses.emplace(typeName, scriptClass);
			}

			SetScriptFieldDefaultData(1, entt::null, typeName);
		}

		MonoScriptUtils::RegisterArrayTypes();
	}

	void MonoScriptEngine::Shutdown()
	{
		MonoGCManager::CollectGarbage(true);

		ShutdownMono();
		s_monoData = nullptr;
	}

	void MonoScriptEngine::OnRuntimeStart(Scene* scene)
	{
		myIsRunning = true;
		s_monoData->sceneContext = scene;

		if (s_monoData->coreClasses.contains("Volt.Scene"))
		{
			mySceneInstance = CreateRef<MonoCoreInstance>(s_monoData->coreClasses.at("Volt.Scene"));
		}

		mono_assembly_setrootdir("Scripts/mono/lib");
		mono_set_assemblies_path("Scripts/mono/lib");
	}

	void MonoScriptEngine::OnRuntimeEnd()
	{
		mono_assembly_setrootdir("Scripts/mono/lib");
		mono_set_assemblies_path("Scripts/mono/lib");

		DoDestroyQueue();

		myIsRunning = false;

		mySceneInstance = nullptr;

		s_monoData->scriptInstances.clear();
		s_monoData->coreInstances.clear();
		s_monoData->scriptEntities.clear();
		s_monoData->sceneContext = nullptr;

		MonoGCManager::CollectGarbage(true);
	}

	void MonoScriptEngine::OnSceneLoaded()
	{
	}

	bool MonoScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_monoData->scriptClasses.contains(fullClassName);
	}

	void MonoScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_monoData->coreData.domain);

		LoadAssembly(s_monoData->coreAssemblyPath);
		LoadAppAssembly(s_monoData->appAssemblyPath);

		s_monoData->coreScriptClass = CreateRef<MonoScriptClass>(s_monoData->coreData.assemblyImage, "Volt", CORE_CLASS_NAME.c_str());
		s_monoData->coreScriptClass->GetMethod(".ctor", 2);

		s_monoData->coreEntityClass = CreateRef<MonoScriptClass>(s_monoData->coreData.assemblyImage, "Volt", ENTITY_CLASS_NAME.c_str());
		s_monoData->coreEntityClass->GetMethod(".ctor", 2);

		LoadAndCreateCoreMonoClasses(s_monoData->coreData.assembly);
		LoadAndCreateMonoClasses(s_monoData->appData.assembly);

		VT_CORE_INFO("[MonoScriptEngine] C# Assembly has been reloaded!");
	}

	void MonoScriptEngine::OnAwakeInstance(UUID instanceId, entt::entity entity, const std::string& fullClassName)
	{
		if (!EntityClassExists(fullClassName))
		{
			return;
		}

		auto scriptEntity = GetOrCreateMonoEntity(entity);

		ScriptParams Params;
		Params.entity = MonoGCManager::GetObjectFromUUID(UUID(static_cast<uint32_t>(entity)));
		Params.scriptId = instanceId;

		auto instance = CreateRef<MonoScriptInstance>(s_monoData->scriptClasses.at(fullClassName), &Params);
		s_monoData->scriptInstances[instanceId] = instance;

		const auto& scriptFields = s_monoData->sceneContext->GetScriptFieldCache().GetCache();

		// Set field values
		if (!scriptFields.contains(instanceId))
		{
			return;
		}

		const auto& fieldMap = scriptFields.at(instanceId);
		for (const auto& [name, fieldInstance] : fieldMap)
		{
			if (fieldInstance->field.type.IsEntity())
			{
				entt::entity fieldEnt = *fieldInstance->data.As<entt::entity>();
				auto fieldEntInstance = GetOrCreateMonoEntity(fieldEnt);

				auto instanceObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
				auto entityObject = MonoGCManager::GetObjectFromHandle(fieldEntInstance->GetHandle());

				if (!instanceObject || !entityObject)
				{
					continue;
				}

				auto field = mono_class_get_field_from_name(instance->GetClass()->GetClass(), name.c_str());
				if (field != nullptr)
				{
					mono_field_set_value(instanceObject, field, entityObject);
				}

				continue;
			}
			else if (fieldInstance->field.type.IsString())
			{
				MonoString* monoString = MonoScriptUtils::GetMonoStringFromString(std::string(fieldInstance->data.As<const char>()));

				auto field = mono_class_get_field_from_name(instance->GetClass()->GetClass(), name.c_str());
				mono_field_set_value(MonoGCManager::GetObjectFromHandle(instance->GetHandle()), field, monoString);

				continue;
			}
			else if (fieldInstance->field.type.IsAsset())
			{
				auto field = mono_class_get_field_from_name(instance->GetClass()->GetClass(), name.c_str());

				auto field_class = mono_class_from_mono_type(mono_field_get_type(field));
				auto ns = mono_class_get_namespace(field_class);
				auto class_name = mono_class_get_name(field_class);

				auto klass = CreateRef<MonoScriptClass>(s_monoData->coreData.assemblyImage, ns, class_name);
				auto gcHandle = InstantiateClass(UUID(), klass->GetClass());

				auto handle = fieldInstance->data.As<AssetHandle>();
				void* args[1] = { handle };

				auto method = klass->GetMethod(".ctor", 1);

				if (method)
				{
					CallMethod(gcHandle, method, args);
				}

				auto instanceObject = MonoGCManager::GetObjectFromHandle(instance->GetHandle());
				auto object = MonoGCManager::GetObjectFromHandle(gcHandle);

				if (!instanceObject || !object)
				{
					continue;
				}

				mono_field_set_value(instanceObject, field, object);
				continue;
			}

			instance->SetField(name, fieldInstance->data.As<const void>());
		}
	}

	void MonoScriptEngine::OnCreateInstance(UUID instanceId, entt::entity entity, const std::string& fullClassName)
	{
		if (s_monoData->scriptInstances.contains(instanceId))
		{
			s_monoData->scriptInstances.at(instanceId)->InvokeOnCreate();
		}
	}

	void MonoScriptEngine::OnUpdateInstance(UUID instanceId, float deltaTime)
	{
		if (s_monoData->scriptInstances.contains(instanceId))
		{
			s_monoData->scriptInstances.at(instanceId)->InvokeOnUpdate(deltaTime);
		}
	}

	void MonoScriptEngine::OnRenderUIInstance(UUID instanceId)
	{
		if (s_monoData->scriptInstances.contains(instanceId))
		{
			s_monoData->scriptInstances.at(instanceId)->InvokeOnRenderUI();
		}
	}

	void MonoScriptEngine::OnDestroyInstance(UUID instanceId)
	{
		s_monoData->scriptDestroyQueue.emplace_back(instanceId);
	}

	void MonoScriptEngine::OnEnableInstance(UUID instanceId)
	{
		if (s_monoData->scriptInstances.contains(instanceId))
		{
			s_monoData->scriptInstances.at(instanceId)->InvokeOnEnable();
		}
	}

	void MonoScriptEngine::OnDisableInstance(UUID instanceId)
	{
		if (s_monoData->scriptInstances.contains(instanceId))
		{
			s_monoData->scriptInstances.at(instanceId)->InvokeOnDisable();
		}
	}

	void MonoScriptEngine::OnUpdateEntity(float deltaTime)
	{
		for (const auto& entity : s_monoData->scriptEntities)
		{
			entity.second->UpdateTimers();
		}
	}

	void MonoScriptEngine::OnUpdate(float deltaTime)
	{
		DoOnCreateQueue();

		if (mySceneInstance)
		{
			mySceneInstance->CallMethod("UpdateSceneTimers");
		}
	}

	void MonoScriptEngine::DoOnAwakeInstance()
	{
		for (const auto& [uuid, instance] : s_monoData->scriptInstances)
		{
			instance->InvokeOnAwake();
		}
	}

	void MonoScriptEngine::DoDestroyQueue()
	{
		for (const auto& instanceId : s_monoData->scriptDestroyQueue)
		{
			if (!s_monoData->scriptEntities.empty())
			{
				s_monoData->scriptEntities.clear();
			}

			if (s_monoData->scriptInstances.contains(instanceId))
			{
				s_monoData->scriptInstances.at(instanceId)->InvokeOnDestroy();
				s_monoData->scriptInstances.erase(instanceId);
			}
		}
	}

	void MonoScriptEngine::DoOnCreateQueue()
	{
		if (!s_monoData->scriptOnCreateQueue.empty())
		{
			for (const auto& sid : s_monoData->scriptOnCreateQueue)
			{
				if (s_monoData->scriptInstances.contains(sid))
				{
					s_monoData->scriptInstances.at(sid)->InvokeOnCreate();
				}
			}
			s_monoData->scriptOnCreateQueue.clear();
		}
	}

	void MonoScriptEngine::QueueOnCreate(UUID instanceId)
	{
		s_monoData->scriptOnCreateQueue.emplace_back(instanceId);
	}

	Scene* MonoScriptEngine::GetSceneContext()
	{
		return s_monoData->sceneContext;
	}

	MonoDomain* MonoScriptEngine::GetAppDomain()
	{
		return s_monoData->coreData.domain;
	}

	Volt::MonoScriptEngine::AssemblyData& MonoScriptEngine::GetCoreAssembly()
	{
		return s_monoData->coreData;
	}

	Volt::MonoScriptEngine::AssemblyData& MonoScriptEngine::GetAppAssembly()
	{
		return s_monoData->appData;
	}

	Ref<MonoScriptClass> MonoScriptEngine::GetScriptClass()
	{
		return s_monoData->coreScriptClass;
	}

	Ref<MonoScriptClass> MonoScriptEngine::GetEntityClass()
	{
		return s_monoData->coreEntityClass;
	}

	const std::unordered_map<std::string, Ref<Volt::MonoScriptClass>>& MonoScriptEngine::GetRegisteredClasses()
	{
		return s_monoData->scriptClasses;
	}

	const std::unordered_map<std::string, Ref<MonoEnum>>& MonoScriptEngine::GetRegisteredEnums()
	{
		return s_monoData->monoEnums;
	}

	const Ref<MonoScriptClass> MonoScriptEngine::GetScriptClass(const std::string& name)
	{
		if (!s_monoData->scriptClasses.contains(name))
		{
			return nullptr;
		}

		return s_monoData->scriptClasses.at(name);
	}

	Ref<MonoScriptInstance> MonoScriptEngine::GetInstanceFromId(UUID instanceId)
	{
		if (!s_monoData->scriptInstances.contains(instanceId))
		{
			return nullptr;
		}

		return s_monoData->scriptInstances.at(instanceId);
	}

	Ref<MonoScriptEntity> MonoScriptEngine::GetEntityFromId(entt::entity id)
	{
		if (!s_monoData->scriptEntities.contains(id))
		{
			return nullptr;
		}

		return s_monoData->scriptEntities.at(id);
	}

	Ref<Volt::MonoScriptEntity> MonoScriptEngine::GetOrCreateMonoEntity(entt::entity id)
	{
		if (!s_monoData->scriptEntities.contains(id))
		{
			std::vector<uint64_t> scriptIds;
			Entity entity = { id, s_monoData->sceneContext };

			if (entity.HasComponent<MonoScriptComponent>())
			{
				for (const auto& uuid : entity.GetComponent<MonoScriptComponent>().scriptIds)
				{
					scriptIds.emplace_back((uint64_t)uuid);
				}
			}

			s_monoData->scriptEntities[id] = CreateRef<MonoScriptEntity>(entity.GetID(), scriptIds, s_monoData->coreEntityClass);
		}

		return s_monoData->scriptEntities.at(id);
	}

	Volt::MonoScriptFieldMap& MonoScriptEngine::GetDefaultScriptFieldMap(std::string fullClassName)
	{
		VT_CORE_ASSERT(EntityClassExists(fullClassName), "Class does not exist!");

		return s_monoData->scriptFieldsDefault[fullClassName];
	}

	void MonoScriptEngine::SetScriptFieldDefaultData(UUID instanceId, entt::entity entity, const std::string& fullClassName)
	{
		if (!EntityClassExists(fullClassName))
		{
			return;
		}

		std::vector<uint64_t> scriptIds;
		Ref<MonoScriptEntity> monoEntity = CreateRef<MonoScriptEntity>(entity, scriptIds, s_monoData->coreEntityClass);

		ScriptParams Params;
		Params.entity = MonoGCManager::GetObjectFromUUID(UUID(static_cast<uint32_t>(entity)));
		Params.scriptId = instanceId;

		auto instance = CreateRef<MonoScriptInstance>(s_monoData->scriptClasses.at(fullClassName), &Params);

		const auto& classFields = Volt::MonoScriptEngine::GetScriptClass(fullClassName)->GetFields();
		auto& defaultEntityFields = Volt::MonoScriptEngine::GetDefaultScriptFieldMap(fullClassName);

		for (const auto& [name, field] : classFields)
		{
			defaultEntityFields[name] = CreateRef<Volt::MonoScriptFieldInstance>();
			defaultEntityFields.at(name)->SetValue(0, field.type.typeSize);
		}

		for (auto& [name, fieldInst] : defaultEntityFields)
		{
			if (fieldInst->field.type.typeIndex == typeid(std::string))
			{
				auto value = instance->GetField<MonoString*>(name);
				auto str = MonoScriptUtils::GetStringFromMonoString(value);

				fieldInst->SetValue(str, str.size());
			}
			else
			{
				fieldInst->SetValue(instance->GetFieldRaw(name), fieldInst->field.type.typeSize);
			}
		}
	}

	void MonoScriptEngine::CallMethod(GCHandle instanceHandle, MonoMethod* method, void** args)
	{
		MonoObject* instance = MonoGCManager::GetObjectFromHandle(instanceHandle);
		if (!instance)
		{
			return;
		}

		MonoObject* exception = nullptr;
		mono_runtime_invoke(method, instance, args, &exception);

		if (exception)
		{
			auto exceptionInfo = Utility::GetExceptionInfo(exception);
			VT_CORE_ERROR("{0}: {1}. Source: {2}, Stack Trace: {3}", exceptionInfo.typeName, exceptionInfo.message, exceptionInfo.source, exceptionInfo.stackTrace);
		}
	}

	void MonoScriptEngine::CallStaticMethod(MonoMethod* method, void** args /*= nullptr*/)
	{
		MonoObject* exception = nullptr;
		mono_runtime_invoke(method, nullptr, args, &exception);

		if (exception)
		{
			auto exceptionInfo = Utility::GetExceptionInfo(exception);
			VT_CORE_ERROR("{0}: {1}. Source: {2}, Stack Trace: {3}", exceptionInfo.typeName, exceptionInfo.message, exceptionInfo.source, exceptionInfo.stackTrace);
		}
	}

	void* MonoScriptEngine::CallMethodImpl(GCHandle instanceHandle, MonoMethod* method, void** args)
	{
		MonoObject* instance = MonoGCManager::GetObjectFromHandle(instanceHandle);
		if (!instance)
		{
			return nullptr;
		}

		MonoObject* exception = nullptr;
		auto monoObject = mono_runtime_invoke(method, instance, args, &exception);

		if (exception)
		{
			auto exceptionInfo = Utility::GetExceptionInfo(exception);
			VT_CORE_ERROR("{0}: {1}. Source: {2}, Stack Trace: {3}", exceptionInfo.typeName, exceptionInfo.message, exceptionInfo.source, exceptionInfo.stackTrace);
			return nullptr;
		}

		return mono_object_unbox(monoObject);
	}

	void MonoScriptEngine::InitializeMono()
	{
		mono_assembly_setrootdir("Scripts/mono/lib");
		mono_set_assemblies_path("Scripts/mono/lib");

		//mono_trace_set_level_string("warning");

		if (!Application::Get().IsRuntime())
		{
			s_monoData->enableDebugging = true;
		}

		if (s_monoData->enableDebugging)
		{
			const char* argv[2] =
			{
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("VoltJITRuntime");
		VT_CORE_ASSERT(rootDomain, "Root domain not initialized!");

		s_monoData->rootDomain = rootDomain;

		if (s_monoData->enableDebugging)
		{
			mono_debug_domain_create(s_monoData->rootDomain);
		}
		mono_thread_set_main(mono_thread_current());
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

	void MonoScriptEngine::ReloadDomain()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_monoData->coreData.domain);

		std::string assemblyName = "VoltScriptRuntime";
		s_monoData->coreData.domain = mono_domain_create_appdomain(assemblyName.data(), nullptr);
		mono_domain_set(s_monoData->coreData.domain, true);
		mono_domain_set_config(s_monoData->coreData.domain, ".", "");
	}

	MonoMethod* MonoScriptEngine::GetScriptConstructor()
	{
		return s_monoData->coreScriptClass->GetMethod(".ctor", 2);
	}

	MonoMethod* MonoScriptEngine::GetEntityConstructor()
	{
		return s_monoData->coreEntityClass->GetMethod(".ctor", 2);
	}

	GCHandle MonoScriptEngine::InstantiateClass(const UUID id, MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_monoData->coreData.domain, monoClass);
		mono_runtime_object_init(instance);

		auto handle = MonoGCManager::AddReferenceToObject(id, instance);
		return handle;
	}

	const std::vector<std::string> MonoScriptEngine::GetReferencedAssembliesName(MonoImage* image)
	{
		const MonoTableInfo* tableInfo = mono_image_get_table_info(image, MONO_TABLE_ASSEMBLYREF);
		int32_t rows = mono_table_info_get_rows(tableInfo);

		std::vector<std::string> names = { "System.Configuration", "Mono.Security", "System.Xml", "System.Net" };
		for (int32_t i = 0; i < rows; i++)
		{
			uint32_t colos[MONO_ASSEMBLYREF_SIZE];
			mono_metadata_decode_row(tableInfo, i, colos, MONO_ASSEMBLYREF_SIZE);

			std::string name = mono_metadata_string_heap(image, colos[MONO_ASSEMBLYREF_NAME]);
			names.push_back(name);
		}

		return names;
	}

	void MonoScriptEngine::LoadReferencedAssemblies(const std::vector<std::string>& assemblyNames)
	{
		const std::filesystem::path baseAssemblyPath = ProjectManager::GetEngineDirectory() / "Scripts" / "mono" / "lib" / "mono" / "4.5";

		for (const auto& name : assemblyNames)
		{
			if (::Utility::StringContains(name, "Volt-ScriptCore"))
			{
				continue;
			}

			if (::Utility::StringContains(name, "mscorlib"))
			{
				continue;
			}

			const auto assemblyPath = baseAssemblyPath / (std::format("{0}.dll", name));
			MonoAssembly* assembly = Utility::LoadCSharpAssembly(assemblyPath, true);

			s_monoData->referencedAssemblies[name] = assembly;
		}
	}
}
