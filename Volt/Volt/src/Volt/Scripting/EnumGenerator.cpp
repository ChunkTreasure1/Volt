#include "vtpch.h"
#include "EnumGenerator.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Utility/FileSystem.h"

#include "Volt/Utility/PremadeCommands.h"

namespace Volt
{
	EnumGenerator::EnumGenerator(const std::string& enumName, const std::string& nameSpace)
		: myName(enumName), myNamespace(nameSpace)
	{}

	void EnumGenerator::AddEnumValue(const std::string& name, int32_t value)
	{
		uint32_t val = (uint32_t)value;

		if (value == -1)
		{
			val = myEnumValueCounter++;
		}

		myEnumValues.emplace_back(name, val);
	}

	void EnumGenerator::WriteToFile(const std::filesystem::path& destination)
	{
		std::stringstream ss;
		if (!myNamespace.empty())
		{
			ss << std::format("namespace {0}\n", myNamespace);
			ss << "{\n";
		}

		ss << std::format("public enum {0} : uint\n", myName);
		ss << "{\n";
		
		for (const auto& val : myEnumValues)
		{
			ss << std::format("{0} = {1},\n", val.name, val.value);
		}

		ss << "}\n";
		if (!myNamespace.empty())
		{
			ss << "}\n";
		}

		if (!FileSystem::Exists(ProjectManager::GetEngineScriptsDirectory()))
		{
			FileSystem::CreateDirectories(ProjectManager::GetEngineScriptsDirectory());
		}

		std::ofstream output{ ProjectManager::GetEngineScriptsDirectory() / destination };
		output << ss.rdbuf();
		output.close();

		Volt::PremadeCommands::RunProjectPremakeCommand();
	}
}
