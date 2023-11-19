#pragma once

#include "Volt/Core/UUID.h"
#include <unordered_map>

extern "C"
{
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Volt
{
	using GCHandle = void*;

	class MonoGCManager
	{
	public:
		static const GCHandle AddReferenceToObject(const UUID64 id, MonoObject* object);
		static void RemoveReferenceToObject(const GCHandle handle);

		static MonoObject* GetObjectFromHandle(const GCHandle handle);
		static MonoObject* GetObjectFromUUID(const UUID64 id);

		static void CollectGarbage(bool waitForThread = false);

	private:
		inline static std::unordered_map<UUID64, GCHandle> myHandles;
	};
}