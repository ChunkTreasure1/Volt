#include "vtpch.h"
#include "BehaviorTreeNode.h"
#include "Volt/BehaviorTree/BehaviorTree.hpp"
#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Utility/StringUtility.h"

#include "Volt/Components/CoreComponents.h"

namespace Volt::BehaviorTree
{
	eNodeStatus Decorator::Run()
	{
		switch (m_type)
		{
			case eDecoratorType::INVERTER:
			{
				switch (m_tree->GetNodeManager().GetNodeFromUUID(m_tree->GetNodeManager().GetLinksFromUUID(m_uuid)[0].m_childID)->Run())
				{
					case eNodeStatus::SUCCESS: return eNodeStatus::FAILURE;
					case eNodeStatus::FAILURE: return eNodeStatus::SUCCESS;
				}

			} break;

			case eDecoratorType::REPEATER:
			{
				if (m_tree->GetRoot() != m_uuid)
				{
					m_tree->SetRoot(m_uuid);
					m_tree->GetNodeManager().GetNodeFromUUID(m_tree->GetNodeManager().GetLinksFromUUID(m_uuid)[0].m_childID)->Run();
				} break;

			case eDecoratorType::REPEAT_UNTIL_FAIL:
			{
				switch (m_tree->GetNodeManager().GetNodeFromUUID(m_tree->GetNodeManager().GetLinksFromUUID(m_uuid)[0].m_childID)->Run())
				{
					case eNodeStatus::SUCCESS:
					{
						m_tree->SetRoot(m_uuid);
						return eNodeStatus::SUCCESS;
					}

					case eNodeStatus::FAILURE:
					{
						m_tree->ResetRoot();
						return eNodeStatus::FAILURE;
					}
				}

			} break;

			case eDecoratorType::SUCCEEDER:
			{
				m_tree->GetNodeManager().GetNodeFromUUID(m_tree->GetNodeManager().GetLinksFromUUID(m_uuid)[0].m_childID)->Run();
				return eNodeStatus::SUCCESS;
			} break;
			case eDecoratorType::IF:
			{
				auto monoElements = ::Utility::SplitStringsByCharacter(m_if, '.');
				if (monoElements.size() != 3)
				{
					VT_LOG(LogVerbosity::Error, "bad decorator function string");
					return eNodeStatus::FAILURE;
				}
				std::string spaceName = monoElements[0] + "." + monoElements[1];

				if (!MonoScriptEngine::GetRegisteredClasses().contains(spaceName))
				{
					VT_LOG(LogVerbosity::Error, "DIN MAMMA");
					return eNodeStatus::FAILURE;
				}

				auto monoClass = MonoScriptEngine::GetScriptClass(spaceName);

				auto monoMethod = monoClass->GetMethod(monoElements[2], 0);
				if (!monoMethod)
				{
					VT_LOG(LogVerbosity::Error, "bad decorator method");
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
						if (MonoScriptEngine::CallMethodE<eNodeStatus>(t, monoMethod) == eNodeStatus::SUCCESS)
						{
							return m_tree->GetNodeManager().GetNodeFromUUID(m_tree->GetNodeManager().GetLinksFromUUID(m_uuid)[0].m_childID)->Run();
						}
						return eNodeStatus::FAILURE;
					}
				}
				// Should nvever be reached
				VT_LOG(LogVerbosity::Error, "Bad Decorator");
				return eNodeStatus::FAILURE;
			} break;
			default:
				VT_LOG(LogVerbosity::Error, "Default decorator failure");
			}

		}
		return eNodeStatus::FAILURE;
	}
}
