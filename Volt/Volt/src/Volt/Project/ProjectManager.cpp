#include "vtpch.h"
#include "ProjectManager.h"

#include "Volt/Utility/FileSystem.h"
#include "Volt/Core/Application.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/StringUtility.h"

#include <AssetSystem/AssetManager.h>

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

namespace Volt
{
	void ProjectManager::SetupProject(const std::filesystem::path projectPath)
	{
		m_currentProject = CreateScope<Project>();

		if (!projectPath.empty())
		{
			m_currentProject->projectDirectory = projectPath.parent_path();
			m_currentProject->projectFilePath = projectPath;
		}
		else
		{
			m_currentProject->projectDirectory = std::filesystem::current_path();
		
			for (const auto& dir : std::filesystem::directory_iterator("./"))
			{
				if (dir.path().extension() == ".vtproj")
				{
					m_currentProject->projectFilePath = dir.path();
					break;
				}
			}
		}

		VT_LOG(Info, "[ProjectManager]: Loading project {0}", projectPath);
		DeserializeProject();

		m_currentEngineDirectory = ::Utility::ReplaceCharacter(FileSystem::GetEnvVariable("VOLT_PATH"), '\\', '/');
		std::filesystem::current_path(m_currentEngineDirectory);
	}

	void ProjectManager::SerializeProject()
	{
		YAMLFileStreamWriter streamWriter{ m_currentProject->projectFilePath };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Project");

		streamWriter.SetKey("EngineVersion", VT_VERSION.ToString());
		streamWriter.SetKey("Name", m_currentProject->name);
		streamWriter.SetKey("CompanyName", m_currentProject->companyName);
		streamWriter.SetKey("AssetsDirectory", m_currentProject->assetsDirectory);
		streamWriter.SetKey("AudioBanksDirectory", m_currentProject->audioBanksDirectory);
		streamWriter.SetKey("IconPath", m_currentProject->iconPath);
		streamWriter.SetKey("CursorPath", m_currentProject->cursorPath);
		streamWriter.SetKey("StartScenePath", m_currentProject->startScenePath);

		streamWriter.EndMap();
		streamWriter.EndMap();
		streamWriter.WriteToDisk();
	}

	void ProjectManager::DeserializeProject()
	{
		YAMLFileStreamReader streamReader{};

		if (!streamReader.OpenFile(m_currentProject->projectFilePath))
		{
			VT_LOG(Error, "[ProjectManager]: Failed to open file: {0}!", m_currentProject->projectFilePath.string());
			return;
		}

		if (!streamReader.HasKey("Project"))
		{
			VT_LOG(Error, "[ProjectManager]: Project file {0} is invalid!", m_currentProject->projectFilePath.string());
			return;
		}

		streamReader.EnterScope("Project");

		m_currentProject->engineVersion = streamReader.ReadAtKey("EngineVersion", std::string(""));
		m_currentProject->name = streamReader.ReadAtKey("Name", std::string("None"));
		m_currentProject->companyName = streamReader.ReadAtKey("CompanyName", std::string("None"));
		m_currentProject->assetsDirectory = streamReader.ReadAtKey("AssetsDirectory", std::filesystem::path("Assets"));
		m_currentProject->audioBanksDirectory = streamReader.ReadAtKey("AudioBanksDirectory", std::filesystem::path("Audio/Banks"));
		m_currentProject->iconPath = streamReader.ReadAtKey("IconPath", std::filesystem::path(""));
		m_currentProject->cursorPath = streamReader.ReadAtKey("CursorPath", std::filesystem::path(""));
		m_currentProject->startScenePath = streamReader.ReadAtKey("StartScene", std::filesystem::path(""));

		streamReader.ExitScope();

		if (!m_currentProject->engineVersion.IsValid() || m_currentProject->engineVersion != Application::Get().GetInfo().version)
		{
			m_currentProject->isDeprecated = true;
			VT_LOG(Error, "[ProjectManager]: The loaded project is deprecated!");
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
		return m_currentProject->isDeprecated ? "./" : m_currentProject->projectDirectory / m_currentProject->assetsDirectory;
	}

	const std::filesystem::path ProjectManager::GetAudioBanksDirectory()
	{
		return m_currentProject->isDeprecated ? "./" : m_currentProject->projectDirectory / m_currentProject->assetsDirectory / m_currentProject->audioBanksDirectory;
	}

	const std::filesystem::path ProjectManager::GetProjectDirectory()
	{
		return m_currentProject->isDeprecated ? "./" : m_currentProject->projectDirectory;
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
		return GetDirectory() / "Binaries" / "Project.dll";
	}

	const std::filesystem::path ProjectManager::GetMonoBinariesDirectory()
	{
		return GetDirectory() / "Binaries";
	}

	const std::filesystem::path ProjectManager::GetOrCreateSettingsDirectory()
	{
		const std::filesystem::path dir = GetDirectory() / "Settings";
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

	const std::filesystem::path& ProjectManager::GetDirectory()
	{
		return m_currentProject->projectDirectory;
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
