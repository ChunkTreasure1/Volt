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
	typedef struct _MonoArray MonoArray;
}

namespace Volt
{
	struct EntityParams
	{
		Wire::EntityId id;
		MonoArray* scriptIds;
	};

	using GCHandle = void*;

	class MonoScriptClass;
	class MonoScriptEntity
	{
	public:
		MonoScriptEntity(const Wire::EntityId& id, const std::vector<uint64_t>& scripts, Ref<MonoScriptClass> klass);
		~MonoScriptEntity();

		void UpdateTimers();

		inline Ref<MonoScriptClass> GetClass() const { return myMonoClass; }
		inline Wire::EntityId GetEntityId() const { return myEntity; }
		inline const GCHandle GetHandle() const { return myHandle; }

	private:
		Ref<MonoScriptClass> myMonoClass;

		MonoMethod* myUpdateTimersMethod = nullptr;

		GCHandle myHandle = nullptr;
		Wire::EntityId myEntity;
	};
}
