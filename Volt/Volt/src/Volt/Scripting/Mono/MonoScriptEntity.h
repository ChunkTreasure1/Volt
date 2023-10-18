#pragma once

#include <entt.hpp>

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
	struct EntityParams
	{
		entt::entity id;
		MonoArray* scriptIds;
	};

	using GCHandle = void*;

	class MonoScriptClass;
	class MonoScriptEntity
	{
	public:
		MonoScriptEntity(const entt::entity& id, const std::vector<uint64_t>& scripts, Ref<MonoScriptClass> klass);
		~MonoScriptEntity();

		void UpdateTimers();

		inline Ref<MonoScriptClass> GetClass() const { return myMonoClass; }
		inline entt::entity GetEntityId() const { return myEntity; }
		inline const GCHandle GetHandle() const { return myHandle; }

	private:
		Ref<MonoScriptClass> myMonoClass;

		MonoMethod* myUpdateTimersMethod = nullptr;

		GCHandle myHandle = nullptr;
		entt::entity myEntity;
	};
}
