#include "vtpch.h"
#include "MonoNetwork.h"

#include "MonoScriptInstance.h"

namespace Volt
{
	std::vector<MonoNetSyncInfo> MonoNetwork::GetSyncData()
	{
		std::vector<MonoNetSyncInfo> data;

		// Set information.

		return data;
	}

	bool MonoNetwork::SetSyncData(const std::vector<MonoNetSyncInfo>& syncData)
	{
		// Init myReplicatedInstances so they match to servers handles.

		return false;
	}

	Ref<Volt::MonoScriptFieldInstance> MonoNetwork::GetRepField(MonoNetHandle handle, const std::string& fieldName)
	{
		if (myReplicatedFieldMaps.contains(handle) && 
			myReplicatedFieldMaps.at(handle).contains(fieldName))
		{
			return myReplicatedFieldMaps.at(handle).at(fieldName);
		}

		return nullptr;
	}

	MonoScriptFieldMap* MonoNetwork::GetRepFields(MonoNetHandle handle)
	{
		if (myReplicatedFieldMaps.contains(handle))
		{
			return &myReplicatedFieldMaps.at(handle);
		}
		return nullptr;
	}

	void MonoNetwork::UpdateField(MonoNetHandle handle, const std::string& fieldName)
	{
		auto instanceExist = myReplicatedInstances.contains(handle);
		auto fieldMapExist = myReplicatedFieldMaps.contains(handle);
		auto fieldExist = myReplicatedFieldMaps.at(handle).contains(fieldName);

		if (instanceExist && fieldMapExist && fieldExist)
		{
			auto data = myReplicatedFieldMaps.at(handle).at(fieldName)->data.As<void>();
			myReplicatedInstances.at(handle).SetField(fieldName, data);
		}
	}

	void MonoNetwork::UpdateFields(MonoNetHandle handle)
	{
		if (myReplicatedFieldMaps.contains(handle))
		{
			for (const auto& [name, field] : myReplicatedFieldMaps.at(handle))
			{
				UpdateField(handle, name);
			}
		}
	}
}
