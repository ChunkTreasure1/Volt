#include "vtcorepch.h"

#include "Volt-Core/Console/ConsoleVariableRegistry.h"

namespace Volt
{
	ConsoleVariableRegistry g_consoleVariableRegistry;

	ConsoleVariableRegistry::ConsoleVariableRegistry()
	{
	}

	ConsoleVariableRegistry::~ConsoleVariableRegistry()
	{
	}

	std::unordered_map<std::string, Ref<RegisteredConsoleVariableBase>>& ConsoleVariableRegistry::GetRegisteredVariables()
	{
		return g_consoleVariableRegistry.m_registeredVariables;
	}

	bool ConsoleVariableRegistry::VariableExists(const std::string& variableName)
	{
		std::string tempVarName = ::Utility::ToLower(std::string(variableName));
		return g_consoleVariableRegistry.m_registeredVariables.contains(tempVarName);
	}

	Weak<RegisteredConsoleVariableBase> ConsoleVariableRegistry::GetVariable(const std::string& variableName)
	{
		const std::string tempVarName = ::Utility::ToLower(variableName);
		return g_consoleVariableRegistry.m_registeredVariables.at(tempVarName);
	}
}
