#pragma once

#include <string>
#include <filesystem>

struct FileFilter
{
	std::string name;
	std::string extensions;
};

class FileSystem
{
public:
	static bool HasEnvironmentVariable(const std::string& key);
	static std::string GetEnvVariable(const std::string& key);

	static bool SetEnvVariable(const std::string& key, const std::string& value);
	static bool SetRegistryValue(const std::string& key, const std::string& value);

	static void StartProcess(const std::filesystem::path& processName, const std::wstring& commandLine = L"");

	static void MoveToRecycleBin(const std::filesystem::path& path);
	static bool ShowDirectoryInExplorer(const std::filesystem::path& aPath);

	static std::filesystem::path PickFolderDialogue();
	static std::filesystem::path OpenFileDialogue(const std::vector<FileFilter>& filters);
	static std::filesystem::path SaveFileDialogue(const std::vector<FileFilter>& filters);
};