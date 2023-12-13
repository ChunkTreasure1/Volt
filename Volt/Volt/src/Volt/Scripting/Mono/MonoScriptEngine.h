#pragma once

#include "Volt/Scripting/Mono/MonoScriptUtils.h"

#include <Volt/Core/UUID.h>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoArray MonoArray;
}

namespace Volt
{
	class Scene;
	class MonoScriptClass;
	class MonoEnum;
	class MonoScriptInstance;
	class MonoCoreInstance;
	class MonoScriptEntity;
	struct MonoScriptFieldInstance;

	using MonoScriptFieldMap = std::unordered_map<std::string, Ref<MonoScriptFieldInstance>>;
	using GCHandle = void*;

	class MonoScriptEngine
	{
	public:
		struct AssemblyData
		{
			MonoDomain* domain = nullptr;
			MonoAssembly* assembly = nullptr;
			MonoImage* assemblyImage = nullptr;
		};

		static void Initialize();
		static void ReloadAssembly();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeEnd();
		static void OnSceneLoaded();

		static bool EntityClassExists(const std::string& fullClassName);

		static void OnAwakeInstance(UUID instanceId, EntityID entity, const std::string& fullClassName);
		static void OnCreateInstance(UUID instanceId, EntityID entity, const std::string& fullClassName);
		static void OnUpdateInstance(UUID instanceId, float deltaTime);
		static void OnRenderUIInstance(UUID instanceId);
		static void OnDestroyInstance(UUID instanceId);
		static void OnEnableInstance(UUID instanceId);
		static void OnDisableInstance(UUID instanceId);

		static void OnUpdateEntity(float deltaTime);
		static void OnUpdate(float deltaTime);

		static void DoOnAwakeInstance();

		static void DoDestroyQueue();
		static void DoOnCreateQueue();
		static void QueueOnCreate(UUID instanceId);

		static Scene* GetSceneContext();
		static MonoDomain* GetAppDomain();

		static AssemblyData& GetCoreAssembly();
		static AssemblyData& GetAppAssembly();

		static Ref<MonoScriptClass> GetScriptClass();
		static Ref<MonoScriptClass> GetEntityClass();

		static const std::unordered_map<std::string, Ref<MonoScriptClass>>& GetRegisteredClasses();
		static const std::unordered_map<std::string, Ref<MonoEnum>>& GetRegisteredEnums();
		static const Ref<MonoScriptClass> GetScriptClass(const std::string& name);

		static Ref<MonoScriptInstance> GetInstanceFromId(UUID instanceId);
		static Ref<MonoScriptEntity> GetEntityFromId(EntityID entityId);
		static Ref<MonoScriptEntity> GetOrCreateMonoEntity(EntityID entity);

		static MonoScriptFieldMap& GetDefaultScriptFieldMap(std::string fullClassName);

		static void SetScriptFieldDefaultData(UUID instanceId, EntityID entity, const std::string& fullClassName);

		static void CallMethod(GCHandle instanceHandle, MonoMethod* method, void** args = nullptr);
		static void CallStaticMethod(MonoMethod* method, void** args = nullptr);

		// #max_stuff: 
		static void* CallMethodImpl(GCHandle instanceHandle, MonoMethod* method, void** args = nullptr);
		template <typename T>
		static T CallMethodE(GCHandle instanceHandle, MonoMethod* method, void** args = nullptr);

		inline static const std::string CORE_CLASS_NAME = "Script";
		inline static const std::string ENTITY_CLASS_NAME = "Entity";
		inline static const size_t MAX_SCRIPTS_PER_ENTITY = 10u;

		static bool IsRunning() { return myIsRunning; }
		static bool NetFieldSetup(MonoScriptClass* monoClass, const std::string& attibuteName, MonoScriptField& out_field);

	private:
		static bool LoadAssembly(const std::filesystem::path& assemblyPath);
		static bool LoadAppAssembly(const std::filesystem::path& assemblyPath);
		
		static void LoadAndCreateCoreMonoClasses(MonoAssembly* assembly);
		static void LoadAndCreateMonoClasses(MonoAssembly* assembly);

		static void RegisterEnumsAndClasses(MonoAssembly* assembly);

		static void InitializeMono();
		static void ShutdownMono();

		static void ReloadDomain();

		static MonoMethod* GetScriptConstructor();
		static MonoMethod* GetEntityConstructor();
		static GCHandle InstantiateClass(const UUID id, MonoClass* monoClass);

		static const std::vector<std::string> GetReferencedAssembliesName(MonoImage* image);
		static void LoadReferencedAssemblies(const std::vector<std::string>& assemblyNames);

		inline static Ref<Volt::MonoCoreInstance> mySceneInstance;

		friend class MonoScriptClass;
		friend class MonoScriptInstance;
		friend class MonoScriptEntity;
		friend class MonoCoreInstance;
		inline static bool myIsRunning = false;

	};

	template<typename T>
	inline T MonoScriptEngine::CallMethodE(GCHandle instanceHandle, MonoMethod* method, void** args)
	{
		auto var = (T*)CallMethodImpl(instanceHandle, method, args);
		if (!var)
		{
			return T{};
		}
		return *var;
	}
}
