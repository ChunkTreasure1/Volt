#include "sbpch.h"
#include "UserSettingsManager.h"

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Utility/EditorLibrary.h"

#include <Volt/Utility/YAMLSerializationHelpers.h>

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

#include <yaml-cpp/yaml.h>

inline static const std::filesystem::path s_userSettingsPath = "User/UserSettings.yaml";
inline static const std::string salt = "epic42069salt";

void UserSettingsManager::LoadUserSettings()
{
	YAMLFileStreamReader streamReader{};
	if (!streamReader.OpenFile(s_userSettingsPath))
	{
		return;
	}

	streamReader.EnterScope("Settings");

	streamReader.ForEach("Windows", [&]() 
	{
		std::string panelTitle = streamReader.ReadAtKey("title", std::string(""));
		bool state = streamReader.ReadAtKey("state", false);

		if (!panelTitle.empty())
		{
			s_editorSettings.panelStates.emplace_back(panelTitle, state);
		}
	});

	streamReader.EnterScope("SceneSettings");
	s_editorSettings.sceneSettings.worldSpace = streamReader.ReadAtKey("worldSpace", true);
	s_editorSettings.sceneSettings.snapToGrid = streamReader.ReadAtKey("snapToGrid", false);
	s_editorSettings.sceneSettings.snapRotation = streamReader.ReadAtKey("snapRotation", false);
	s_editorSettings.sceneSettings.snapScale = streamReader.ReadAtKey("snapScale", false);
	s_editorSettings.sceneSettings.showGizmos = streamReader.ReadAtKey("showGizmos", true);
	s_editorSettings.sceneSettings.use16by9 = streamReader.ReadAtKey("use16by9", false);
	s_editorSettings.sceneSettings.fullscreenOnPlay = streamReader.ReadAtKey("fullscreenOnPlay", false);
	s_editorSettings.sceneSettings.gridEnabled = streamReader.ReadAtKey("gridEnabled", true);
	s_editorSettings.sceneSettings.gridSnapValue = streamReader.ReadAtKey("gridSnapValue", 0.5f);
	s_editorSettings.sceneSettings.rotationSnapValue = streamReader.ReadAtKey("rotationSnapValue", 0.45f);
	s_editorSettings.sceneSettings.scaleSnapValue = streamReader.ReadAtKey("scaleSnapValue", 0.1f);
	s_editorSettings.sceneSettings.showLightSpheres = streamReader.ReadAtKey("showLightSpheres", true);
	s_editorSettings.sceneSettings.showEntityGizmos = streamReader.ReadAtKey("showEntityGizmos", true);
	s_editorSettings.sceneSettings.showBoundingSpheres = streamReader.ReadAtKey("showBoundingSpheres", false);

	s_editorSettings.sceneSettings.colliderViewMode = (ColliderViewMode)streamReader.ReadAtKey("colliderViewMode", 0u);
	s_editorSettings.sceneSettings.showEnvironmentProbes = streamReader.ReadAtKey("showEnvironmentProbes", false);
	s_editorSettings.sceneSettings.navMeshViewMode = (NavMeshViewMode)streamReader.ReadAtKey("navMeshViewMode", 0u);

	s_editorSettings.sceneSettings.defaultOpenScene = streamReader.ReadAtKey("defaultOpenScene", Volt::Asset::Null());
	streamReader.ExitScope();

	streamReader.EnterScope("VersionControlSettings");
	s_editorSettings.versionControlSettings.server = streamReader.ReadAtKey("server", std::string("localhost:1666"));
	s_editorSettings.versionControlSettings.user = streamReader.ReadAtKey("user", std::string());
	s_editorSettings.versionControlSettings.password = streamReader.ReadAtKey("password", std::string());
	s_editorSettings.versionControlSettings.workspace = streamReader.ReadAtKey("workspace", std::string());
	s_editorSettings.versionControlSettings.stream = streamReader.ReadAtKey("stream", std::string());
	streamReader.ExitScope();

	streamReader.EnterScope("ExternalToolsSettings");
	s_editorSettings.externalToolsSettings.customExternalScriptEditor = streamReader.ReadAtKey("customExternalScriptEditor", std::filesystem::path("C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe"));
	streamReader.ExitScope();

	if (s_editorSettings.externalToolsSettings.customExternalScriptEditor.empty())
	{
		s_editorSettings.externalToolsSettings.customExternalScriptEditor = std::filesystem::path("C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe");
	}

	streamReader.EnterScope("NavMeshBuildSettings");

	s_editorSettings.navmeshBuildSettings.maxAgents = streamReader.ReadAtKey("maxAgents", 100);
	s_editorSettings.navmeshBuildSettings.agentHeight = streamReader.ReadAtKey("agentHeight", 2.f);
	s_editorSettings.navmeshBuildSettings.agentRadius = streamReader.ReadAtKey("agentRadius", 0.6f);
	s_editorSettings.navmeshBuildSettings.agentMaxClimb = streamReader.ReadAtKey("agentMaxClimb", 0.9f);
	s_editorSettings.navmeshBuildSettings.agentMaxSlope = streamReader.ReadAtKey("agentMaxSlope", 0.45f);
	
	s_editorSettings.navmeshBuildSettings.cellSize = streamReader.ReadAtKey("cellSize", 30.f);
	s_editorSettings.navmeshBuildSettings.cellHeight = streamReader.ReadAtKey("cellHeight", 1.f);
	s_editorSettings.navmeshBuildSettings.regionMinSize = streamReader.ReadAtKey("regionMinSize", 8.f);
	s_editorSettings.navmeshBuildSettings.regionMergeSize = streamReader.ReadAtKey("regionMergeSize", 20.f);
	s_editorSettings.navmeshBuildSettings.edgeMaxLen = streamReader.ReadAtKey("edgeMaxLen", 12.f);
	s_editorSettings.navmeshBuildSettings.edgeMaxError = streamReader.ReadAtKey("edgeMaxError", 1.3f);
	s_editorSettings.navmeshBuildSettings.vertsPerPoly = streamReader.ReadAtKey("vertsPerPoly", 6.f);
	s_editorSettings.navmeshBuildSettings.detailSampleDist = streamReader.ReadAtKey("detailSampleDist", 6.f);
	s_editorSettings.navmeshBuildSettings.detailSampleMaxError = streamReader.ReadAtKey("detailSampleMaxError", 1.f);
	s_editorSettings.navmeshBuildSettings.partitionType = streamReader.ReadAtKey("partitionType", 0);
	s_editorSettings.navmeshBuildSettings.useTileCache = streamReader.ReadAtKey("useTileCache", false);
	s_editorSettings.navmeshBuildSettings.useAutoBaking = streamReader.ReadAtKey("useAutoBaking", true);

	streamReader.ExitScope();

	streamReader.EnterScope("NetworkSettings");
	s_editorSettings.networkSettings.enableNetworking = streamReader.ReadAtKey("enableNetworking", false);
	streamReader.ExitScope();

	streamReader.EnterScope("AssetBrowserSettings");
	s_editorSettings.assetBrowserSettings.thumbnailSize = streamReader.ReadAtKey("thumbnailSize", 85.f);
	streamReader.ExitScope();

	streamReader.ExitScope();
}

void UserSettingsManager::SaveUserSettings()
{
	YAMLFileStreamWriter streamWriter{ s_userSettingsPath };
	streamWriter.BeginMap();
	streamWriter.BeginMapNamned("Settings");
	{
		streamWriter.BeginSequence("Windows");
		for (const auto& window : EditorLibrary::GetPanels())
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("title", window.editorWindow->GetTitle());
			streamWriter.SetKey("state", window.editorWindow->IsOpen());
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginMapNamned("SceneSettings");
		{
			streamWriter.SetKey("worldSpace", s_editorSettings.sceneSettings.worldSpace);
			streamWriter.SetKey("snapToGrid", s_editorSettings.sceneSettings.snapToGrid);
			streamWriter.SetKey("snapRotation", s_editorSettings.sceneSettings.snapRotation);
			streamWriter.SetKey("snapScale", s_editorSettings.sceneSettings.snapScale);
			streamWriter.SetKey("showGizmos", s_editorSettings.sceneSettings.showGizmos);
			streamWriter.SetKey("use16by9", s_editorSettings.sceneSettings.use16by9);
			streamWriter.SetKey("fullscreenOnPlay", s_editorSettings.sceneSettings.fullscreenOnPlay);
			streamWriter.SetKey("gridEnabled", s_editorSettings.sceneSettings.gridEnabled);
			streamWriter.SetKey("gridSnapValue", s_editorSettings.sceneSettings.gridSnapValue);
			streamWriter.SetKey("rotationSnapValue", s_editorSettings.sceneSettings.rotationSnapValue);
			streamWriter.SetKey("scaleSnapValue", s_editorSettings.sceneSettings.scaleSnapValue);
			streamWriter.SetKey("defaultOpenScene", s_editorSettings.sceneSettings.defaultOpenScene);

			streamWriter.SetKey("showLightSpheres", s_editorSettings.sceneSettings.showLightSpheres);
			streamWriter.SetKey("showEntityGizmos", s_editorSettings.sceneSettings.showEntityGizmos);
			streamWriter.SetKey("showBoundingSpheres", s_editorSettings.sceneSettings.showBoundingSpheres);
			streamWriter.SetKey("colliderViewMode", (uint32_t)s_editorSettings.sceneSettings.colliderViewMode);
			streamWriter.SetKey("showEnvironmentProbes", s_editorSettings.sceneSettings.showEnvironmentProbes);
			streamWriter.SetKey("navMeshViewMode", (uint32_t)s_editorSettings.sceneSettings.navMeshViewMode);
		}
		streamWriter.EndMap();
	
		streamWriter.BeginMapNamned("VersionControlSettings");
		{
			streamWriter.SetKey("server", s_editorSettings.versionControlSettings.server);
			streamWriter.SetKey("user", s_editorSettings.versionControlSettings.user);
			streamWriter.SetKey("password", s_editorSettings.versionControlSettings.password);
			streamWriter.SetKey("workspace", s_editorSettings.versionControlSettings.workspace);
			streamWriter.SetKey("stream", s_editorSettings.versionControlSettings.stream);
		}
		streamWriter.EndMap();

		streamWriter.BeginMapNamned("ExternalToolsSettings");
		{
			streamWriter.SetKey("customExternalScriptEditor", s_editorSettings.externalToolsSettings.customExternalScriptEditor);
		}
		streamWriter.EndMap();

		streamWriter.BeginMapNamned("NavMeshBuildSettings");
		{
			streamWriter.SetKey("maxAgents", s_editorSettings.navmeshBuildSettings.maxAgents);
			streamWriter.SetKey("agentHeight", s_editorSettings.navmeshBuildSettings.agentHeight);
			streamWriter.SetKey("agentRadius", s_editorSettings.navmeshBuildSettings.agentRadius);
			streamWriter.SetKey("agentMaxClimb", s_editorSettings.navmeshBuildSettings.agentMaxClimb);
			streamWriter.SetKey("agentMaxSlope", s_editorSettings.navmeshBuildSettings.agentMaxSlope);

			streamWriter.SetKey("cellSize", s_editorSettings.navmeshBuildSettings.cellSize);
			streamWriter.SetKey("cellHeight", s_editorSettings.navmeshBuildSettings.cellHeight);
			streamWriter.SetKey("regionMinSize", s_editorSettings.navmeshBuildSettings.regionMinSize);
			streamWriter.SetKey("regionMergeSize", s_editorSettings.navmeshBuildSettings.regionMergeSize);
			streamWriter.SetKey("edgeMaxLen", s_editorSettings.navmeshBuildSettings.edgeMaxLen);
			streamWriter.SetKey("edgeMaxError", s_editorSettings.navmeshBuildSettings.edgeMaxError);
			streamWriter.SetKey("vertsPerPoly", s_editorSettings.navmeshBuildSettings.vertsPerPoly);
			streamWriter.SetKey("detailSampleDist", s_editorSettings.navmeshBuildSettings.detailSampleDist);
			streamWriter.SetKey("detailSampleMaxError", s_editorSettings.navmeshBuildSettings.detailSampleMaxError);
			streamWriter.SetKey("partitionType", s_editorSettings.navmeshBuildSettings.partitionType);
			streamWriter.SetKey("useTileCache", s_editorSettings.navmeshBuildSettings.useTileCache);
			streamWriter.SetKey("useAutoBaking", s_editorSettings.navmeshBuildSettings.useAutoBaking);
		}
		streamWriter.EndMap();
	
		streamWriter.BeginMapNamned("NetworkSettings");
		{
			streamWriter.SetKey("enableNetworking", s_editorSettings.networkSettings.enableNetworking);
		}
		streamWriter.EndMap();
	
		streamWriter.BeginMapNamned("AssetBrowserSettings");
		{
			streamWriter.SetKey("thumbnailSize", s_editorSettings.assetBrowserSettings.thumbnailSize);
		}
		streamWriter.EndMap();
	}
	streamWriter.EndMap();
	streamWriter.EndMap();

	streamWriter.WriteToDisk();
}

void UserSettingsManager::SetupPanels()
{
	for (const auto& state : s_editorSettings.panelStates)
	{
		auto editor = EditorLibrary::GetPanel(state.panelName);
		if (editor)
		{
			if (state.isOpen)
			{
				editor->Open();
			}
			else
			{
				editor->Close();
			}
		}
	}
}
