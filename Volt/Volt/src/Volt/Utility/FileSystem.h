#pragma once

#include <filesystem>

class FileSystem
{
public:
	static std::filesystem::path GetEnginePath()
	{
		return "Engine";
	}

	static std::filesystem::path GetShadersPath()
	{
		return "Engine/Shaders";
	}

	static std::filesystem::path GetDebugDumpPath()
	{
		return "User/DebugDumps/";
	}

	static std::filesystem::path GetPhysicsSettingsPath()
	{
		return "Engine/Physics/PhysicsSettings.yaml";
	}

	static std::filesystem::path GetPhysicsLayersPath()
	{
		return "Engine/Physics/PhysicsLayers.yaml";
	}

	static bool IsWriteable(const std::filesystem::path& aPath)
	{
		std::filesystem::file_status status = std::filesystem::status(aPath);
		return (status.permissions() & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
	}

	static bool Copy(const std::filesystem::path& aSource, const std::filesystem::path& aDestination)
	{
		if (!Exists(aSource))
		{
			return false;
		}

		std::filesystem::copy(aSource, aDestination);
		return true;
	}

	static bool CopyFileTo(const std::filesystem::path& aSource, const std::filesystem::path& aDestDir)
	{
		if (!Exists(aDestDir))
		{
			return false;
		}

		std::filesystem::path newPath = aDestDir / aSource.filename();

		if (!Exists(newPath))
		{
			Copy(aSource, newPath);
			return true;
		}

		return false;
	}

	static bool Exists(const std::filesystem::path& aPath)
	{
		return std::filesystem::exists(aPath);
	}

	static bool Remove(const std::filesystem::path& aPath)
	{
		return std::filesystem::remove_all(aPath);
	}

	static void MoveToRecycleBin(const std::filesystem::path& path);
	static bool Rename(const std::filesystem::path& aPath, const std::string& aName)
	{
		if (!Exists(aPath))
		{
			return false;
		}

		const std::filesystem::path newPath = aPath.parent_path() / (aName + aPath.extension().string());
		std::filesystem::rename(aPath, newPath);

		return true;
	}

	static bool Move(const std::filesystem::path& file, const std::filesystem::path& destinationFolder)
	{
		if (!Exists(file))
		{
			return false;
		}

		const std::filesystem::path newPath = destinationFolder / file.filename();
		std::filesystem::rename(file, newPath);

		return true;
	}

	static bool MoveFolder(const std::filesystem::path& sourceFolder, const std::filesystem::path& destinationFolder)
	{
		if (!Exists(sourceFolder))
		{
			return false;
		}

		std::filesystem::rename(sourceFolder, destinationFolder);

		return true;
	}

	static bool CreateFolder(const std::filesystem::path& folder)
	{
		if (Exists(folder))
		{
			return false;
		}

		std::filesystem::create_directories(folder);

		return true;
	}

	static bool ShowDirectoryInExplorer(const std::filesystem::path& aPath);

	static bool ShowFileInExplorer(const std::filesystem::path& aPath)
	{
		auto absolutePath = std::filesystem::canonical(aPath);
		if (!Exists(absolutePath))
		{
			return false;
		}

		std::string cmd = "explorer.exe /select,\"" + absolutePath.string() + "\"";
		system(cmd.c_str());

		return true;
	}

	static bool OpenFileExternally(const std::filesystem::path& aPath);

	static std::filesystem::path OpenFile(const char* filter);
	static std::filesystem::path OpenFolder();
	static std::filesystem::path SaveFile(const char* filter);
	static std::filesystem::path GetDocumentsPath();

	static bool HasEnvironmentVariable(const std::string& key);
	static bool SetEnvVariable(const std::string& key, const std::string& value);
	static bool SetRegistryValue(const std::string& key, const std::string& value);

	static std::string GetEnvVariable(const std::string& key);
	static std::string GetCurrentUserName();

	static void StartProcess(const std::filesystem::path& processName);

	static void RunCommand(const std::string& aCommand)
	{
		system(aCommand.c_str());
	}
};