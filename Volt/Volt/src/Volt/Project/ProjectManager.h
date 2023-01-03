#pragma once

namespace Volt
{
	class ProjectManager
	{
	public:
		static void SetupWorkingDirectory();
		static void SetupProject(const std::filesystem::path projectPath);

		static const std::filesystem::path GetAssetsPath();
		static const std::filesystem::path GetAssetRegistryPath();
		static const std::filesystem::path GetCachePath();
		static const std::filesystem::path GetPathRelativeToProject(const std::filesystem::path& path);
		static const std::filesystem::path GetMonoAssemblyPath();
		static const std::filesystem::path& GetDirectory();


	private:
		ProjectManager() = delete;

		inline static std::filesystem::path myCurrentProjectDirectory;
		inline static std::filesystem::path myCurrentProjectFile;
	};
}