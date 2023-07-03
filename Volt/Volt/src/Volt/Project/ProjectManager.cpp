#include "vtpch.h"
#include "ProjectManager.h"

#include "Volt/Utility/FileSystem.h"
#include "Volt/Core/Application.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/SerializationMacros.h"

namespace Volt
{
	void ProjectManager::SetupWorkingDirectory()
	{
		if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
		{
			const std::string pathEnv = FileSystem::GetEnvVariable("VOLT_PATH");
			std::filesystem::current_path(pathEnv);
		}
	}

	void ProjectManager::SetupProject(const std::filesystem::path projectPath)
	{
		myCurrentProject = CreateScope<Project>();

		if (!projectPath.empty())
		{
			myCurrentProject->projectDirectory = projectPath.parent_path();
			myCurrentProject->projectFilePath = projectPath;

		}
		else
		{
			myCurrentProject->projectDirectory = "./";
		
			for (const auto& dir : std::filesystem::directory_iterator("./"))
			{
				if (dir.path().extension() == ".vtproj")
				{
					myCurrentProject->projectFilePath = dir.path();
					break;
				}
			}

			if (myCurrentProject->projectFilePath.empty())
			{
				throw std::runtime_error("Project File not found!");
			}
		}

		LoadProjectInfo();

		if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
		{
			myCurrentEngineDirectory = FileSystem::GetEnvVariable("VOLT_PATH");
		}
	}

	void ProjectManager::LoadProjectInfo()
	{
		std::ifstream file(myCurrentProject->projectFilePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", myCurrentProject->projectFilePath.string().c_str());
			return;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", myCurrentProject->projectFilePath, e.what());
			return;
		}

		YAML::Node rootProjectNode = root["Project"];

		VT_DESERIALIZE_PROPERTY(Name, myCurrentProject->name, rootProjectNode, std::string("None"));
		VT_DESERIALIZE_PROPERTY(CompanyName, myCurrentProject->companyName, rootProjectNode, std::string("None"));
		VT_DESERIALIZE_PROPERTY(AssetsPath, myCurrentProject->assetsDirectory, rootProjectNode, std::filesystem::path("Assets"));
		VT_DESERIALIZE_PROPERTY(AudioBanksPath, myCurrentProject->audioBanksDirectory, rootProjectNode, std::filesystem::path("Audio/Banks"));
		VT_DESERIALIZE_PROPERTY(IconPath, myCurrentProject->iconPath, rootProjectNode, std::filesystem::path(""));
		VT_DESERIALIZE_PROPERTY(CursorPath, myCurrentProject->cursorPath, rootProjectNode, std::filesystem::path(""));
		VT_DESERIALIZE_PROPERTY(StartScene, myCurrentProject->startScenePath, rootProjectNode, std::filesystem::path(""));
	}

	const std::filesystem::path ProjectManager::GetEngineScriptsDirectory()
	{
		return GetAssetsDirectory() / "Scripts/Internal";
	}

	const std::filesystem::path ProjectManager::GetAssetsDirectory()
	{
		return myCurrentProject->projectDirectory / myCurrentProject->assetsDirectory;
	}

	const std::filesystem::path ProjectManager::GetAudioBanksDirectory()
	{
		return myCurrentProject->projectDirectory / myCurrentProject->assetsDirectory / myCurrentProject->audioBanksDirectory;
	}

	const std::filesystem::path ProjectManager::GetProjectDirectory()
	{
		return myCurrentProject->projectDirectory;
	}

	const std::filesystem::path ProjectManager::GetEngineDirectory()
	{
		return myCurrentEngineDirectory;
	}

	const std::filesystem::path ProjectManager::GetPathRelativeToEngine(const std::filesystem::path& path)
	{
		return std::filesystem::relative(path, myCurrentEngineDirectory);
	}

	const std::filesystem::path ProjectManager::GetCachePath()
	{
		return myCurrentProject->projectDirectory / myCurrentProject->assetsDirectory / "Cache";
	}

	const std::filesystem::path ProjectManager::GetPathRelativeToProject(const std::filesystem::path& path)
	{
		return std::filesystem::relative(path, myCurrentProject->projectDirectory);
	}

	const std::filesystem::path ProjectManager::GetMonoAssemblyPath()
	{
		return GetDirectory() / "Binaries" / "Project.dll";
	}

	const std::filesystem::path& ProjectManager::GetDirectory()
	{
		return myCurrentProject->projectDirectory;
	}

	const Project& ProjectManager::GetProject()
	{
		return *myCurrentProject;
	}
}
