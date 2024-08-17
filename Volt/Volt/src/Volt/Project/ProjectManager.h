#pragma once

#include "Volt/Project/Project.h"
#include "Volt/Utility/Version.h"

#include <CoreUtilities/Core.h>

VT_DECLARE_LOG_CATEGORY(LogProject, LogVerbosity::Trace);

namespace Volt
{
	class PluginRegistry;

	class ProjectManager
	{
	public:
		static void LoadProject(const std::filesystem::path projectPath, PluginRegistry& pluginRegistry);

		static void SerializeProject();

		static const std::filesystem::path GetEngineScriptsDirectory();
		static const std::filesystem::path GetEngineShaderIncludeDirectory();
		static const std::filesystem::path GetAssetsDirectory();
		static const std::filesystem::path GetAudioBanksDirectory();
		static const std::filesystem::path GetProjectDirectory();
		static const std::filesystem::path GetEngineDirectory();
		static const std::filesystem::path GetPathRelativeToEngine(const std::filesystem::path& path);
		static const std::filesystem::path GetPathRelativeToProject(const std::filesystem::path& path);
		static const std::filesystem::path GetCachePath();
		static const std::filesystem::path GetMonoAssemblyPath();
		static const std::filesystem::path GetMonoBinariesDirectory();
		static const std::filesystem::path GetOrCreateSettingsDirectory();
		static const std::filesystem::path GetPhysicsSettingsPath();
		static const std::filesystem::path GetPhysicsLayersPath();
		static const std::filesystem::path& GetRootDirectory();

		static const bool IsCurrentProjectDeprecated();
		static const bool AreCurrentProjectMetaFilesDeprecated();

		static const Project& GetProject();
		static void OnProjectUpgraded();

	private:
		static void DeserializeProject(PluginRegistry& pluginRegistry);

		ProjectManager() = delete;

		inline static std::filesystem::path m_currentEngineDirectory;

		inline static Scope<Project> m_currentProject;
	};
}
