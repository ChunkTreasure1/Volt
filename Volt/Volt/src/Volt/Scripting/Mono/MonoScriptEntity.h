#pragma once

#include "Volt/Scene/EntityID.h"

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
		EntityID id;
		MonoArray* scriptIds;
	};

	using GCHandle = void*;

	class MonoScriptClass;
	class MonoScriptEntity
	{
	public:
		MonoScriptEntity(const EntityID& id, const std::vector<uint64_t>& scripts, Ref<MonoScriptClass> klass);
		~MonoScriptEntity();

		void UpdateTimers();

		inline Ref<MonoScriptClass> GetClass() const { return myMonoClass; }
		inline EntityID GetEntityId() const { return myEntity; }
		inline const GCHandle GetHandle() const { return myHandle; }

	private:
		Ref<MonoScriptClass> myMonoClass;

		MonoMethod* myUpdateTimersMethod = nullptr;

		GCHandle myHandle = nullptr;
		EntityID myEntity;
	};
}
