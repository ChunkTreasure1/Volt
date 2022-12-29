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
		const auto cannonicalPath = std::filesystem::canonical(aPath);

		std::filesystem::file_status status = std::filesystem::status(cannonicalPath);
		return (status.permissions() & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
	}

	static bool Copy(const std::filesystem::path& aSource, const std::filesystem::path& aDestination)
	{
		const auto cannonicalSource = std::filesystem::canonical(aSource);
		const auto cannonicalDest = std::filesystem::canonical(aDestination);

		if (!Exists(cannonicalSource))
		{
			return false;
		}

		std::filesystem::copy(cannonicalSource, cannonicalDest);
		return true;
	}

	static bool CopyFileTo(const std::filesystem::path& aSource, const std::filesystem::path& aDestDir)
	{
		const auto cannonicalSource = std::filesystem::canonical(aSource);
		const auto cannonicalDest = std::filesystem::canonical(aDestDir);

		if (!Exists(cannonicalDest))
		{
			return false;
		}

		std::filesystem::path newPath = cannonicalDest / cannonicalSource.filename();

		if (!Exists(newPath))
		{
			Copy(cannonicalSource, newPath);
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
		const auto cannonicalPath = std::filesystem::canonical(aPath);
		return std::filesystem::remove_all(cannonicalPath);
	}

	static void MoveToRecycleBin(const std::filesystem::path& path);
	static bool Rename(const std::filesystem::path& aPath, const std::string& aName)
	{
		const auto cannonicalPath = std::filesystem::canonical(aPath);

		if (!Exists(cannonicalPath))
		{
			return false;
		}

		const std::filesystem::path newPath = cannonicalPath.parent_path() / (aName + cannonicalPath.extension().string());
		std::filesystem::rename(cannonicalPath, newPath);

		return true;
	}

	static bool Move(const std::filesystem::path& file, const std::filesystem::path& destinationFolder)
	{
		const auto canonicalFile = std::filesystem::canonical(file);
		const auto canonicalDest = std::filesystem::canonical(destinationFolder);

		if (!Exists(canonicalFile))
		{
			return false;
		}

		const std::filesystem::path newPath = canonicalDest/ file.filename();
		std::filesystem::rename(canonicalFile, newPath);

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

	inline static std::atomic_bool globalIsOpenSaveFileOpen = false;
};