#include "vtpch.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Utility/FileSystem.h"
#include "Volt/Core/Application.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/StringUtility.h"

#include "Volt/PluginSystem/PluginRegistry.h"

#include <AssetSystem/AssetManager.h>

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

VT_DEFINE_LOG_CATEGORY(LogProject);

namespace Volt
{
	void ProjectManager::LoadProject(const std::filesystem::path projectPath, PluginRegistry& pluginRegistry)
	{
		m_currentProject = CreateScope<Project>();

		if (!projectPath.empty())
		{
			m_currentProject->rootDirectory = projectPath.parent_path();
			m_currentProject->filepath = projectPath;
		}
		else
		{
			m_currentProject->rootDirectory = std::filesystem::current_path();
		
			for (const auto& dir : std::filesystem::directory_iterator("./"))
			{
				if (dir.path().extension() == ".vtproj")
				{
					m_currentProject->filepath = dir.path();
					break;
				}
			}
		}

		m_currentEngineDirectory = ::Utility::ReplaceCharacter(FileSystem::GetEnvVariable("VOLT_PATH"), '\\', '/');

		pluginRegistry.FindAndRegisterPluginsInDirectory(m_currentProject->rootDirectory / "Plugins");
		pluginRegistry.FindAndRegisterPluginsInDirectory(m_currentEngineDirectory / "Plugins");

		VT_LOGC(Info, LogProject, "Loading project {0}", projectPath);
		DeserializeProject(pluginRegistry);

		std::filesystem::current_path(m_currentEngineDirectory);
	}

	void ProjectManager::SerializeProject()
	{
		YAMLFileStreamWriter streamWriter{ m_currentProject->filepath };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Project");

		streamWriter.SetKey("EngineVersion", VT_VERSION.ToString());
		streamWriter.SetKey("Name", m_currentProject->name);
		streamWriter.SetKey("CompanyName", m_currentProject->companyName);
		streamWriter.SetKey("AssetsDirectory", m_currentProject->assetsDirectory);
		streamWriter.SetKey("AudioBanksDirectory", m_currentProject->audioDirectory);
		streamWriter.SetKey("IconPath", m_currentProject->iconFilepath);
		streamWriter.SetKey("CursorPath", m_currentProject->cursorFilepath);
		streamWriter.SetKey("StartScenePath", m_currentProject->startSceneFilepath);

		streamWriter.EndMap();
		streamWriter.EndMap();
		streamWriter.WriteToDisk();
	}

	void ProjectManager::DeserializeProject(PluginRegistry& pluginRegistry)
	{
		YAMLFileStreamReader streamReader{};

		if (!streamReader.OpenFile(m_currentProject->filepath))
		{
			VT_LOGC(Error, LogProject, "Failed to open file: {0}!", m_currentProject->filepath.string());
			return;
		}

		if (!streamReader.HasKey("Project"))
		{
			VT_LOGC(Error, LogProject, "Project file {0} is invalid!", m_currentProject->filepath.string());
			return;
		}

		streamReader.EnterScope("Project");

		m_currentProject->engineVersion = streamReader.ReadAtKey("EngineVersion", std::string(""));
		m_currentProject->name = streamReader.ReadAtKey("Name", std::string("None"));
		m_currentProject->companyName = streamReader.ReadAtKey("CompanyName", std::string("None"));
		m_currentProject->assetsDirectory = streamReader.ReadAtKey("AssetsDirectory", std::filesystem::path("Assets"));
		m_currentProject->audioDirectory = streamReader.ReadAtKey("AudioBanksDirectory", std::filesystem::path("Audio/Banks"));
		m_currentProject->iconFilepath = streamReader.ReadAtKey("IconPath", std::filesystem::path(""));
		m_currentProject->cursorFilepath = streamReader.ReadAtKey("CursorPath", std::filesystem::path(""));
		m_currentProject->startSceneFilepath = streamReader.ReadAtKey("StartScene", std::filesystem::path(""));

		streamReader.ForEach("Plugins", [&]() 
		{
			const std::string pluginName = streamReader.ReadValue<std::string>();
			const auto& definition = pluginRegistry.GetPluginDefinitionByName(pluginName);
			if (definition.guid != VoltGUID::Null())
			{
				m_currentProject->pluginDefinitions.emplace_back(definition);
			}
			else
			{
				VT_LOGC(Warning, LogProject, "Plugin with name {} does not exist!", pluginName);
			}
		});
		streamReader.ExitScope();

		if (!m_currentProject->engineVersion.IsValid() || m_currentProject->engineVersion != Application::Get().GetInfo().version)
		{
			m_currentProject->isDeprecated = true;
			VT_LOGC(Error, LogProject, "The loaded project is deprecated!");
		}
	}

	const std::filesystem::path ProjectManager::GetEngineScriptsDirectory()
	{
		return GetAssetsDirectory() / "Scripts/Internal";
	}

	const std::filesystem::path ProjectManager::GetEngineShaderIncludeDirectory()
	{
		return "Engine/Shaders/Source/Includes";
	}

	const std::filesystem::path ProjectManager::GetAssetsDirectory()
	{
		return m_currentProject->isDeprecated ? "./" : m_currentProject->rootDirectory / m_currentProject->assetsDirectory;
	}

	const std::filesystem::path ProjectManager::GetAudioBanksDirectory()
	{
		return m_currentProject->isDeprecated ? "./" : m_currentProject->rootDirectory / m_currentProject->audioDirectory;
	}

	const std::filesystem::path ProjectManager::GetProjectDirectory()
	{
		return m_currentProject->isDeprecated ? "./" : m_currentProject->rootDirectory;
	}

	const std::filesystem::path ProjectManager::GetEngineDirectory()
	{
		return m_currentEngineDirectory;
	}

	const std::filesystem::path ProjectManager::GetPathRelativeToEngine(const std::filesystem::path& path)
	{
		return std::filesystem::relative(path, m_currentEngineDirectory);
	}

	const std::filesystem::path ProjectManager::GetCachePath()
	{
		return GetProjectDirectory() / GetAssetsDirectory() / "Cache";
	}

	const std::filesystem::path ProjectManager::GetPathRelativeToProject(const std::filesystem::path& path)
	{
		return std::filesystem::relative(path, GetProjectDirectory());
	}

	const std::filesystem::path ProjectManager::GetMonoAssemblyPath()
	{
		return GetRootDirectory() / "Binaries" / "Project.dll";
	}

	const std::filesystem::path ProjectManager::GetMonoBinariesDirectory()
	{
		return GetRootDirectory() / "Binaries";
	}

	const std::filesystem::path ProjectManager::GetOrCreateSettingsDirectory()
	{
		const std::filesystem::path dir = GetRootDirectory() / "Settings";
		if (!std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}

		return dir;
	}

	const std::filesystem::path ProjectManager::GetPhysicsSettingsPath()
	{
		return GetOrCreateSettingsDirectory() / "PhysicsSettings.yaml";
	}

	const std::filesystem::path ProjectManager::GetPhysicsLayersPath()
	{
		return GetOrCreateSettingsDirectory() / "PhysicsLayers.yaml";
	}

	const std::filesystem::path& ProjectManager::GetRootDirectory()
	{
		return m_currentProject->rootDirectory;
	}

	const bool ProjectManager::IsCurrentProjectDeprecated()
	{
		return m_currentProject->isDeprecated;
	}

	const bool ProjectManager::AreCurrentProjectMetaFilesDeprecated()
	{
		return m_currentProject->engineVersion < Version::Create(0, 1, 1);
	}

	const Project& ProjectManager::GetProject()
	{
		return *m_currentProject;
	}

	void ProjectManager::OnProjectUpgraded()
	{
		m_currentProject->isDeprecated = false;
	}
}
