#include "vtpch.h"
#include "CallMonoMethod.h"

#include "Volt/Components/Components.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Utility/StringUtility.h"


std::vector<void*> Volt::GetMonoArguments(const std::vector<uint8_t>& in_data)
{
	if (in_data.empty()) return std::vector<void*>();

	uint8_t argCount = in_data[0];
	std::vector<void*> data;
	data.resize(argCount);

	size_t index = 1;
	int currentArg = 0;
	while (currentArg < argCount)
	{
		size_t size = in_data[index];
		index++;
		data[currentArg] = (void*)&in_data[index];
		index += size;
		currentArg++;
	}

	return data;
}

bool Volt::CallMonoMethod(const Entity& in_entity, const std::string& in_method, const std::vector<uint8_t>& in_data)
{
	if (in_method == "") return false;
	auto monoElements = Utils::SplitStringsByCharacter(in_method, '.');
	if (monoElements.size() != 3)
	{
		VT_CORE_ERROR("Incorrect method format in CallMonoMethod");
		return false;
	}

	std::string spaceName = monoElements[0] + "." + monoElements[1];

	auto monoClass = MonoScriptEngine::GetScriptClass(spaceName);

	auto args = GetMonoArguments(in_data);

	auto monoMethod = monoClass->GetMethod(monoElements[2], args.size());
	if (!monoMethod)
	{
		VT_CORE_ERROR("monoMethod null");
		return false;
	}

	auto scriptsVector = in_entity.GetComponent<MonoScriptComponent>().scriptIds;
	Ref<MonoScriptInstance> scrInstance = nullptr;
	for (auto scrID : scriptsVector)
	{
		scrInstance = MonoScriptEngine::GetInstanceFromId(scrID);
		if (!scrInstance)
			continue;
		if (monoClass == scrInstance->GetClass())
		{
			auto t = MonoScriptEngine::GetInstanceFromId(scrID)->GetHandle();

			if (args.empty())
			{
				MonoScriptEngine::CallMethod(t, monoMethod);
				return true;
			}
			MonoScriptEngine::CallMethod(t, monoMethod, args.data());
			return true;
		}
	}
	VT_CORE_ERROR("some error occurd");
	return false;
}