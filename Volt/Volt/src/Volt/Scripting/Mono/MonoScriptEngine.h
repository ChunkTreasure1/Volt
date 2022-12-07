#pragma once

#include <Wire/Entity.h>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoImage MonoImage;
}

namespace Volt
{
	class Scene;
	class MonoScriptClass;
	class MonoScriptInstance;
	struct MonoScriptFieldInstance;

	using MonoScriptFieldMap = std::unordered_map<std::string, MonoScriptFieldInstance>;

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
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeEnd();

		static bool EntityClassExists(const std::string& fullClassName);
		static void ReloadAssembly();

		static void OnCreateEntityInstance(Wire::EntityId entityId, const std::string& fullClassName);
		static void OnUpdateEntityInstance(Wire::EntityId entityId, float deltaTime);
		static void OnDestroyEntityInstance(Wire::EntityId entityId);

		static Scene* GetSceneContext();
		static MonoDomain* GetAppDomain();
		
		static Ref<MonoScriptClass> GetEntityMonoClass();
		static const Ref<MonoScriptClass> GetScriptClass(const std::string& name);

		static Ref<MonoScriptInstance> GetInstanceFromEntityId(Wire::EntityId entityId);

		static MonoScriptFieldMap& GetScriptFieldMap(Wire::EntityId entityId);

	private:
		static AssemblyData LoadAssembly(const std::filesystem::path& assemblyPath, const std::string& name, bool createDomain);
		static void LoadAndCreateMonoClasses(MonoAssembly* assembly);

		static void InitializeMono();
		static void ShutdownMono();

		static MonoMethod* GetEntityConstructor();
		static MonoObject* InstantiateClass(MonoClass* monoClass);

		friend class MonoScriptClass;
		friend class MonoScriptInstance;
	};
}