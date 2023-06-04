#pragma once

namespace Volt
{
	struct Project
	{
		std::string name;
		std::filesystem::path projectFilePath;
		std::filesystem::path projectDirectory;

		std::filesystem::path audioBanksDirectory;
		std::filesystem::path assetsDirectory;

		std::filesystem::path cursorPath;
		std::filesystem::path iconPath;
		std::filesystem::path startScenePath;
	};

	class ProjectManager
	{
	public:
		static void SetupWorkingDirectory();
		static void SetupProject(const std::filesystem::path projectPath);

		static const std::filesystem::path GetEngineScriptsDirectory();
		static const std::filesystem::path GetAssetsDirectory();
		static const std::filesystem::path GetAudioBanksDirectory();
		static const std::filesystem::path GetProjectDirectory();
		static const std::filesystem::path GetEngineDirectory();
		static const std::filesystem::path GetPathRelativeToEngine(const std::filesystem::path& path);
		static const std::filesystem::path GetPathRelativeToProject(const std::filesystem::path& path);
		static const std::filesystem::path GetCachePath();
		static const std::filesystem::path GetMonoAssemblyPath();
		static const std::filesystem::path& GetDirectory();

		static const Project& GetProject();


	private:
		static void LoadProjectInfo();

		ProjectManager() = delete;

		inline static std::filesystem::path myCurrentEngineDirectory;
	
		inline static Scope<Project> myCurrentProject;
	};
}
