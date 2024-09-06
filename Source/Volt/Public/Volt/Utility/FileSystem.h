#pragma once

#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/FileSystem.h>

#include <filesystem>

namespace FileSystem
{
	static std::filesystem::path GetEnginePath()
	{
		return "Engine";
	}

	static std::filesystem::path GetScriptsPath()
	{
		return "Scripts";
	}

	static std::filesystem::path GetShadersPath()
	{
		return "Engine/Shaders";
	}

	static std::filesystem::path GetDebugDumpPath()
	{
		return "User/DebugDumps/";
	}

	extern bool HasEnvironmentVariable(const std::string& key);
	extern bool SetEnvVariable(const std::string& key, const std::string& value);
	extern bool SetRegistryValue(const std::string& key, const std::string& value);

	extern std::string GetEnvVariable(const std::string& key);
	extern std::string GetCurrentUserName();

	extern void StartProcess(const std::filesystem::path& processName);

	static bool RunCommand(const std::string& aCommand)
	{
		return system(aCommand.c_str());
	}
};
