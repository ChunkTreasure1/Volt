#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/UUID.h"
#include "Volt/Core/Buffer.h"

#include "MonoScriptEngine.h"

#include <unordered_map>

namespace Volt
{
	typedef uint32_t MonoNetHandle;

	struct MonoNetSyncInfo
	{
		MonoNetHandle handle;
		// Other stuff to make sure the ScriptInstances match on server and client.
	};

	class MonoNetwork
	{
	public:
		MonoNetwork() = delete;

		// Might make a better solution later
		static std::vector<MonoNetSyncInfo> GetSyncData();
		static bool SetSyncData(const std::vector<MonoNetSyncInfo>& syncData);

		static Ref<Volt::MonoScriptFieldInstance> GetRepField(MonoNetHandle handle, const std::string& fieldName);
		static MonoScriptFieldMap* GetRepFields(MonoNetHandle handle);

		static void UpdateField(MonoNetHandle handle, const std::string& fieldName);
		static void UpdateFields(MonoNetHandle handle);

	private:
		inline static std::unordered_map<MonoNetHandle, MonoScriptInstance> myReplicatedInstances;
		inline static std::unordered_map<MonoNetHandle, MonoScriptFieldMap> myReplicatedFieldMaps;
	};
}
