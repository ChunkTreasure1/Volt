#pragma once

#include "Volt/Utility/Version.h"

namespace Volt
{
	struct Project
	{
		std::string name;
		std::string companyName;
		Version engineVersion;
		std::filesystem::path projectFilePath;
		std::filesystem::path projectDirectory;

		std::filesystem::path audioBanksDirectory;
		std::filesystem::path assetsDirectory;

		std::filesystem::path cursorPath;
		std::filesystem::path iconPath;
		std::filesystem::path startScenePath;

		bool isDeprecated = false;
	};

	class ProjectManager
	{
	public:
		static void SetupProject(const std::filesystem::path projectPath);

		static void SerializeProject();
		static void DeserializeProject();

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

		static const bool IsCurrentProjectDeprecated();
		static const bool AreCurrentProjectMetaFilesDeprecated();

		static const Project& GetProject();
		static void OnProjectUpgraded();

	private:

		ProjectManager() = delete;

		inline static std::filesystem::path m_currentEngineDirectory;

		inline static Scope<Project> m_currentProject;
	};
}
