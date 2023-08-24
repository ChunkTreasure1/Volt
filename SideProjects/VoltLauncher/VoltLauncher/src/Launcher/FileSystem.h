#pragma once

#include <string>
#include <filesystem>

class FileSystem
{
public:
	static bool HasEnvironmentVariable(const std::string& key);
	static std::string GetEnvVariable(const std::string& key);

	static bool SetEnvVariable(const std::string& key, const std::string& value);
	static bool SetRegistryValue(const std::string& key, const std::string& value);

	static void StartProcess(const std::filesystem::path& processName);

	static std::filesystem::path PickFolderDialogue();
};