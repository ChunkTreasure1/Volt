#include "vtpch.h"
#include "MonoGCManager.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/object.h>

namespace Volt
{
	const GCHandle MonoGCManager::AddReferenceToObject(const UUID id, MonoObject* object)
	{
		auto handle = mono_gchandle_new_v2(object, false);
		myHandles.emplace(id, handle);

		return handle;
	}

	void MonoGCManager::RemoveReferenceToObject(const GCHandle handle)
	{
		auto obj = mono_gchandle_get_target_v2(handle);
		if (obj)
		{
			mono_gchandle_free_v2(handle);
		}
	}

	MonoObject* MonoGCManager::GetObjectFromHandle(const GCHandle handle)
	{
		auto obj = mono_gchandle_get_target_v2(handle);
		if (!obj || !mono_object_get_vtable(obj))
		{
			return nullptr;
		}

		return obj;
	}

	MonoObject* MonoGCManager::GetObjectFromUUID(const UUID id)
	{
		if (!myHandles.contains(id))
		{
			return nullptr;
		}

		auto handle = myHandles.at(id);

		auto obj = mono_gchandle_get_target_v2(handle);
		if (!obj || !mono_object_get_vtable(obj))
		{
			return nullptr;
		}

		return obj;
	}

	void MonoGCManager::CollectGarbage(bool waitForThread)
	{
		for (const auto& [id, handle] : myHandles)
		{
			mono_gchandle_free_v2(handle);
		}

		myHandles.clear();

		// goopty loopty doo, garbage gone are you
		mono_gc_collect(mono_gc_max_generation());
		if (waitForThread)
		{
			while (mono_gc_pending_finalizers());
		}
	}
}
