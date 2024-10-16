#pragma once

#include "Volt-Core/Project/Project.h"
#include "Volt-Core/Config.h"

#include <SubSystem/SubSystem.h>
#include <CoreUtilities/Core.h>

VT_DECLARE_LOG_CATEGORY_EXPORT(VTCORE_API, LogProject, LogVerbosity::Trace);

namespace Volt
{
	class PluginRegistry;

	class VTCORE_API ProjectManager : public SubSystem
	{
	public:
		ProjectManager();
		~ProjectManager();

		ProjectManager(const ProjectManager& other) = delete;
		ProjectManager& operator=(const ProjectManager& other) = delete;

		void LoadProject(const std::filesystem::path projectPath, PluginRegistry& pluginRegistry);
		void SerializeProject();

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

		VT_DECLARE_SUBSYSTEM("{A1ED2D3C-2994-4B81-984D-23E25B9193F6}"_guid)

	private:
		inline static ProjectManager* s_instance = nullptr;

		void DeserializeProject(PluginRegistry& pluginRegistry);

		std::filesystem::path m_currentEngineDirectory;
		Scope<Project> m_currentProject;
	};
}
