#include "vtpch.h"
#include "BehaviorTreeNode.h"
#include "../BehaviorTree.hpp"
#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Utility/StringUtility.h"
#include "Volt/Components/CoreComponents.h"
namespace Volt::BehaviorTree
{
	eNodeStatus Leaf::Run()
	{
		auto monoElements = ::Utility::SplitStringsByCharacter(m_monoScriptFunctonName, '.');
		if (monoElements.size() != 3)
		{
			VT_CORE_ERROR("Bad leaf");
			return eNodeStatus::FAILURE;
		}

		std::string spaceName = monoElements[0] + "." + monoElements[1];

		auto monoClass = MonoScriptEngine::GetScriptClass(spaceName);

		auto monoMethod = monoClass->GetMethod(monoElements[2], 0);
		if (!monoMethod)
		{
			VT_CORE_ERROR("Bad Leaf node");
			return eNodeStatus::FAILURE;
		}

		auto scriptsVector = m_tree->GetEntity().GetComponent<MonoScriptComponent>().scriptIds;
		Ref<MonoScriptInstance> scrInstance = nullptr;
		for (auto scrID : scriptsVector)
		{
			scrInstance = MonoScriptEngine::GetInstanceFromId(scrID);
			if (!scrInstance)
				continue;
			if (monoClass == scrInstance->GetClass())
			{
				auto t = MonoScriptEngine::GetInstanceFromId(scrID)->GetHandle();
				return MonoScriptEngine::CallMethodE<eNodeStatus>(t, monoMethod);
			}
		}
		// Should nvever be reached
		VT_CORE_ERROR("Bad Leaf");
		return eNodeStatus::FAILURE;
	}
}
