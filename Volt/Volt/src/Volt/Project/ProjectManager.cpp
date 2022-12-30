#include "vtpch.h"
#include "ProjectManager.h"

#include "Volt/Utility/FileSystem.h"

namespace Volt
{
	void ProjectManager::SetupWorkingDirectory()
	{
		if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
		{
			const std::string pathEnv = FileSystem::GetEnvVariable("VOLT_PATH");
			if (!pathEnv.empty())
			{
				std::filesystem::current_path(pathEnv);
			}
		}
	}
	
	void ProjectManager::SetupProject(const std::filesystem::path projectPath)
	{
		if (projectPath.empty())
		{
			const auto workingDir = std::filesystem::current_path();
			myCurrentProjectDirectory = workingDir;
		}
		else
		{
			myCurrentProjectDirectory = projectPath.parent_path();
			myCurrentProjectFile = projectPath;
		}
	}

	const std::filesystem::path ProjectManager::GetAssetsPath()
	{
		return myCurrentProjectDirectory / "Assets";
	}

	const std::filesystem::path ProjectManager::GetAssetRegistryPath()
	{
		return GetAssetsPath() / "AssetRegistry.vtreg";
	}

	const std::filesystem::path ProjectManager::GetCachePath()
	{
		return myCurrentProjectDirectory / "Assets" / "Cache";
	}

	const std::filesystem::path ProjectManager::GetPathRelativeToProject(const std::filesystem::path& path)
	{
		return std::filesystem::relative(path, myCurrentProjectDirectory);
	}

	const std::filesystem::path ProjectManager::GetMonoAssemblyPath()
	{
		return GetPath() / "Binaries" / "Project.dll";
	}

	const std::filesystem::path& ProjectManager::GetPath()
	{
		return myCurrentProjectDirectory;
	}
}