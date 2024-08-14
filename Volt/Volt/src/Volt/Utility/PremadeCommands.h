#pragma once
#include "FileSystem.h"
#include "Volt/Project/ProjectManager.h"

#include <fstream>

namespace Volt
{
	class PremadeCommands
	{
	public:
		PremadeCommands() = delete;

		static bool RunProjectPremakeCommand()
		{
			auto temp = std::filesystem::current_path();
			std::filesystem::current_path(Volt::ProjectManager::GetProjectDirectory());
			std::string command = "start /B /wait %VOLT_PATH%\\vendor\\premake5.exe vs2022";
			FileSystem::RunCommand(command);
			std::filesystem::current_path(temp);
			return true;
		};

		static bool RunOpenProjectSolutionCommand()
		{
			RunProjectPremakeCommand();

			auto temp = std::filesystem::current_path();
			std::filesystem::current_path(Volt::ProjectManager::GetProjectDirectory());
			std::string command = "start Project.sln";
			FileSystem::RunCommand(command);
			std::filesystem::current_path(temp);
			return true;
		};

		static bool RunOpenVSFileCommand(std::filesystem::path externalEditor, std::filesystem::path filepath)
		{
			std::ifstream infile(externalEditor.c_str());
			if (infile.fail() || externalEditor.filename() != "devenv.exe")
			{
				return false; 
			}

			auto temp = std::filesystem::current_path();
			std::filesystem::current_path(Volt::ProjectManager::GetProjectDirectory());
			std::string command = "\"" + externalEditor.string() + "\" /edit " + filepath.string();
			FileSystem::RunCommand(command);
			std::filesystem::current_path(temp);
			return true;
		};

		static bool RunBuildCSProjectCommand(std::filesystem::path externalEditor)
		{
			std::ifstream infile(externalEditor.c_str());
			if (infile.fail() || externalEditor.filename() != "devenv.exe")
			{
				return false;
			}
			RunProjectPremakeCommand();

			auto temp = std::filesystem::current_path();
			std::filesystem::current_path(Volt::ProjectManager::GetProjectDirectory());
			std::string command = "\"" + externalEditor.string() + "\" Project.sln /Rebuild DIST /project Project /projectconfig DIST";
			auto buildStatus = FileSystem::RunCommand(command);
			std::filesystem::current_path(temp);
			return (buildStatus) ? false : true;
		};
	};
}
