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
	streamReader.EnterScope("Windows");

	for (const auto& window : EditorLibrary::GetPanels())
	{
		if (streamReader.HasKey(window.editorWindow->GetTitle()))
		{
			bool value = streamReader.ReadAtKey(window.editorWindow->GetTitle(), false);
			if (value)
			{
				window.editorWindow->Open();
			}
			else
			{
				window.editorWindow->Close();
			}
		}
	}

	streamReader.ExitScope();

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
	s_editorSettings.sceneSettings.lowMemoryUsage = streamReader.ReadAtKey("lowMemoryUsage", false);

	s_editorSettings.sceneSettings.lastOpenScene = streamReader.ReadAtKey("lastOpenScene", std::string());
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
	// #TODO_Ivar: Reimplement
	//YAML::Emitter out;
	//out << YAML::BeginMap;
	//out << YAML::Key << "Settings" << YAML::Value;
	//{
	//	out << YAML::BeginMap;
	//	out << YAML::Key << "Windows" << YAML::BeginSeq;
	//	for (const auto& window : EditorLibrary::GetPanels())
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY_STRING(window.editorWindow->GetTitle(), window.editorWindow->IsOpen(), out);
	//		out << YAML::EndMap;
	//	}
	//	out << YAML::EndSeq;

	//	out << YAML::Key << "SceneSettings" << YAML::Value;
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY(worldSpace, s_editorSettings.sceneSettings.worldSpace, out);
	//		VT_SERIALIZE_PROPERTY(snapToGrid, s_editorSettings.sceneSettings.snapToGrid, out);
	//		VT_SERIALIZE_PROPERTY(snapRotation, s_editorSettings.sceneSettings.snapRotation, out);
	//		VT_SERIALIZE_PROPERTY(snapScale, s_editorSettings.sceneSettings.snapScale, out);
	//		VT_SERIALIZE_PROPERTY(showGizmos, s_editorSettings.sceneSettings.showGizmos, out);
	//		VT_SERIALIZE_PROPERTY(use16by9, s_editorSettings.sceneSettings.use16by9, out);
	//		VT_SERIALIZE_PROPERTY(fullscreenOnPlay, s_editorSettings.sceneSettings.fullscreenOnPlay, out);
	//		VT_SERIALIZE_PROPERTY(gridEnabled, s_editorSettings.sceneSettings.gridEnabled, out);
	//		VT_SERIALIZE_PROPERTY(gridSnapValue, s_editorSettings.sceneSettings.gridSnapValue, out);
	//		VT_SERIALIZE_PROPERTY(rotationSnapValue, s_editorSettings.sceneSettings.rotationSnapValue, out);
	//		VT_SERIALIZE_PROPERTY(scaleSnapValue, s_editorSettings.sceneSettings.scaleSnapValue, out);
	//		VT_SERIALIZE_PROPERTY(lastOpenScene, s_editorSettings.sceneSettings.lastOpenScene.string(), out);

	//		VT_SERIALIZE_PROPERTY(showLightSpheres, s_editorSettings.sceneSettings.showLightSpheres, out);
	//		VT_SERIALIZE_PROPERTY(showEntityGizmos, s_editorSettings.sceneSettings.showEntityGizmos, out);
	//		VT_SERIALIZE_PROPERTY(showBoundingSpheres, s_editorSettings.sceneSettings.showBoundingSpheres, out);
	//		VT_SERIALIZE_PROPERTY(colliderViewMode, (uint32_t)s_editorSettings.sceneSettings.colliderViewMode, out);
	//		VT_SERIALIZE_PROPERTY(showEnvironmentProbes, s_editorSettings.sceneSettings.showEnvironmentProbes, out);
	//		VT_SERIALIZE_PROPERTY(navMeshViewMode, (uint32_t)s_editorSettings.sceneSettings.navMeshViewMode, out);
	//		VT_SERIALIZE_PROPERTY(lowMemoryUsage, s_editorSettings.sceneSettings.lowMemoryUsage, out);
	//		out << YAML::EndMap;
	//	}

	//	out << YAML::Key << "VersionControlSettings" << YAML::Value;
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY(server, s_editorSettings.versionControlSettings.server, out);
	//		VT_SERIALIZE_PROPERTY(user, s_editorSettings.versionControlSettings.user, out);
	//		VT_SERIALIZE_PROPERTY(password, s_editorSettings.versionControlSettings.password, out);
	//		VT_SERIALIZE_PROPERTY(workspace, s_editorSettings.versionControlSettings.workspace, out);
	//		VT_SERIALIZE_PROPERTY(stream, s_editorSettings.versionControlSettings.stream, out);
	//		out << YAML::EndMap;
	//	}

	//	out << YAML::Key << "ExternalToolsSettings" << YAML::Value;
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY(customExternalScriptEditor, s_editorSettings.externalToolsSettings.customExternalScriptEditor.string(), out);
	//		out << YAML::EndMap;
	//	}

	//	out << YAML::Key << "NavMeshBuildSettings" << YAML::Value;
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY(maxAgents, s_editorSettings.navmeshBuildSettings.maxAgents, out);
	//		VT_SERIALIZE_PROPERTY(agentHeight, s_editorSettings.navmeshBuildSettings.agentHeight, out);
	//		VT_SERIALIZE_PROPERTY(agentRadius, s_editorSettings.navmeshBuildSettings.agentRadius, out);
	//		VT_SERIALIZE_PROPERTY(agentMaxClimb, s_editorSettings.navmeshBuildSettings.agentMaxClimb, out);
	//		VT_SERIALIZE_PROPERTY(agentMaxSlope, s_editorSettings.navmeshBuildSettings.agentMaxSlope, out);

	//		VT_SERIALIZE_PROPERTY(cellSize, s_editorSettings.navmeshBuildSettings.cellSize, out);
	//		VT_SERIALIZE_PROPERTY(cellHeight, s_editorSettings.navmeshBuildSettings.cellHeight, out);
	//		VT_SERIALIZE_PROPERTY(regionMinSize, s_editorSettings.navmeshBuildSettings.regionMinSize, out);
	//		VT_SERIALIZE_PROPERTY(regionMergeSize, s_editorSettings.navmeshBuildSettings.regionMergeSize, out);
	//		VT_SERIALIZE_PROPERTY(edgeMaxLen, s_editorSettings.navmeshBuildSettings.edgeMaxLen, out);
	//		VT_SERIALIZE_PROPERTY(edgeMaxError, s_editorSettings.navmeshBuildSettings.edgeMaxError, out);
	//		VT_SERIALIZE_PROPERTY(vertsPerPoly, s_editorSettings.navmeshBuildSettings.vertsPerPoly, out);
	//		VT_SERIALIZE_PROPERTY(detailSampleDist, s_editorSettings.navmeshBuildSettings.detailSampleDist, out);
	//		VT_SERIALIZE_PROPERTY(detailSampleMaxError, s_editorSettings.navmeshBuildSettings.detailSampleMaxError, out);
	//		VT_SERIALIZE_PROPERTY(partitionType, s_editorSettings.navmeshBuildSettings.partitionType, out);
	//		VT_SERIALIZE_PROPERTY(useTileCache, s_editorSettings.navmeshBuildSettings.useTileCache, out);
	//		VT_SERIALIZE_PROPERTY(useAutoBaking, s_editorSettings.navmeshBuildSettings.useAutoBaking, out);
	//		out << YAML::EndMap;
	//	}

	//	out << YAML::Key << "NetworkSettings" << YAML::Value;
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY(enableNetworking, s_editorSettings.networkSettings.enableNetworking, out);
	//		out << YAML::EndMap;
	//	}

	//	out << YAML::Key << "AssetBrowserSettings" << YAML::Value;
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY(thumbnailSize, s_editorSettings.assetBrowserSettings.thumbnailSize, out);
	//		out << YAML::EndMap;
	//	}

	//	out << YAML::EndMap;
	//}
	//out << YAML::EndMap;

	//std::ofstream fout(s_userSettingsPath);
	//fout << out.c_str();
	//fout.close();
}
