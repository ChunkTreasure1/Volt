#include "sbpch.h"
#include "UserSettingsManager.h"

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Utility/EditorLibrary.h"

#include <Volt/Utility/YAMLSerializationHelpers.h>
#include <Volt/Utility/SerializationMacros.h>

#include <yaml-cpp/yaml.h>

inline static const std::filesystem::path s_userSettingsPath = "User/UserSettings.yaml";
inline static const std::string salt = "epic42069salt";

void UserSettingsManager::LoadUserSettings()
{
	std::ifstream file(s_userSettingsPath);
	if (!file.is_open())
	{
		return;
	}

	std::stringstream sstream;
	sstream << file.rdbuf();
	file.close();

	YAML::Node root = YAML::Load(sstream.str());
	YAML::Node settingsNode = root["Settings"];

	YAML::Node windowsNode = settingsNode["Windows"];
	for (const auto& window : EditorLibrary::GetPanels())
	{
		if (windowsNode[window.editorWindow->GetTitle()])
		{
			bool value = windowsNode[window.editorWindow->GetTitle()].as<bool>();

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

	YAML::Node sceneSettingsNode = settingsNode["SceneSettings"];
	VT_DESERIALIZE_PROPERTY(worldSpace, s_editorSettings.sceneSettings.worldSpace, sceneSettingsNode, true);
	VT_DESERIALIZE_PROPERTY(snapToGrid, s_editorSettings.sceneSettings.snapToGrid, sceneSettingsNode, false);
	VT_DESERIALIZE_PROPERTY(snapRotation, s_editorSettings.sceneSettings.snapRotation, sceneSettingsNode, false);
	VT_DESERIALIZE_PROPERTY(snapScale, s_editorSettings.sceneSettings.snapScale, sceneSettingsNode, false);
	VT_DESERIALIZE_PROPERTY(showGizmos, s_editorSettings.sceneSettings.showGizmos, sceneSettingsNode, true);
	VT_DESERIALIZE_PROPERTY(use16by9, s_editorSettings.sceneSettings.use16by9, sceneSettingsNode, false);
	VT_DESERIALIZE_PROPERTY(fullscreenOnPlay, s_editorSettings.sceneSettings.fullscreenOnPlay, sceneSettingsNode, false);
	VT_DESERIALIZE_PROPERTY(gridEnabled, s_editorSettings.sceneSettings.gridEnabled, sceneSettingsNode, true);
	VT_DESERIALIZE_PROPERTY(gridSnapValue, s_editorSettings.sceneSettings.gridSnapValue, sceneSettingsNode, 50.f);
	VT_DESERIALIZE_PROPERTY(rotationSnapValue, s_editorSettings.sceneSettings.rotationSnapValue, sceneSettingsNode, 45.f);
	VT_DESERIALIZE_PROPERTY(scaleSnapValue, s_editorSettings.sceneSettings.scaleSnapValue, sceneSettingsNode, 0.1f);

	VT_DESERIALIZE_PROPERTY(showLightSpheres, s_editorSettings.sceneSettings.showLightSpheres, sceneSettingsNode, true);
	VT_DESERIALIZE_PROPERTY(showEntityGizmos, s_editorSettings.sceneSettings.showEntityGizmos, sceneSettingsNode, true);
	VT_DESERIALIZE_PROPERTY(showBoundingSpheres, s_editorSettings.sceneSettings.showBoundingSpheres, sceneSettingsNode, false);

	VT_DESERIALIZE_PROPERTY(colliderViewMode, *(uint32_t*)&s_editorSettings.sceneSettings.colliderViewMode, sceneSettingsNode, 0u);
	VT_DESERIALIZE_PROPERTY(showEnvironmentProbes, s_editorSettings.sceneSettings.showEnvironmentProbes, sceneSettingsNode, false);
	VT_DESERIALIZE_PROPERTY(navMeshViewMode, *(uint32_t*)&s_editorSettings.sceneSettings.navMeshViewMode, sceneSettingsNode, 0u);
	VT_DESERIALIZE_PROPERTY(lowMemoryUsage, s_editorSettings.sceneSettings.lowMemoryUsage, sceneSettingsNode, false);

	std::string sceneInput;
	VT_DESERIALIZE_PROPERTY(lastOpenScene, sceneInput, sceneSettingsNode, std::string());
	s_editorSettings.sceneSettings.lastOpenScene = sceneInput;

	YAML::Node versionControlnode = settingsNode["VersionControlSettings"];
	VT_DESERIALIZE_PROPERTY(server, s_editorSettings.versionControlSettings.server, versionControlnode, std::string("localhost:1666"));
	VT_DESERIALIZE_PROPERTY(user, s_editorSettings.versionControlSettings.user, versionControlnode, std::string());
	VT_DESERIALIZE_PROPERTY(password, s_editorSettings.versionControlSettings.password, versionControlnode, std::string());
	VT_DESERIALIZE_PROPERTY(workspace, s_editorSettings.versionControlSettings.workspace, versionControlnode, std::string());
	VT_DESERIALIZE_PROPERTY(stream, s_editorSettings.versionControlSettings.stream, versionControlnode, std::string());

	YAML::Node externalToolsnode = settingsNode["ExternalToolsSettings"];
	VT_DESERIALIZE_PROPERTY(customExternalScriptEditor, s_editorSettings.externalToolsSettings.customExternalScriptEditor, externalToolsnode, std::filesystem::path("C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe"));

	if (s_editorSettings.externalToolsSettings.customExternalScriptEditor.empty())
	{
		s_editorSettings.externalToolsSettings.customExternalScriptEditor = std::filesystem::path("C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe");
	}

	YAML::Node navMeshBuildnode = settingsNode["NavMeshBuildSettings"];
	VT_DESERIALIZE_PROPERTY(maxAgents, s_editorSettings.navmeshBuildSettings.maxAgents, navMeshBuildnode, 100);
	VT_DESERIALIZE_PROPERTY(agentHeight, s_editorSettings.navmeshBuildSettings.agentHeight, navMeshBuildnode, 200.f);
	VT_DESERIALIZE_PROPERTY(agentRadius, s_editorSettings.navmeshBuildSettings.agentRadius, navMeshBuildnode, 60.f);
	VT_DESERIALIZE_PROPERTY(agentMaxClimb, s_editorSettings.navmeshBuildSettings.agentMaxClimb, navMeshBuildnode, 90.f);
	VT_DESERIALIZE_PROPERTY(agentMaxSlope, s_editorSettings.navmeshBuildSettings.agentMaxSlope, navMeshBuildnode, 45.f);

	VT_DESERIALIZE_PROPERTY(cellSize, s_editorSettings.navmeshBuildSettings.cellSize, navMeshBuildnode, 30.f);
	VT_DESERIALIZE_PROPERTY(cellHeight, s_editorSettings.navmeshBuildSettings.cellHeight, navMeshBuildnode, 1.f);
	VT_DESERIALIZE_PROPERTY(regionMinSize, s_editorSettings.navmeshBuildSettings.regionMinSize, navMeshBuildnode, 8.f);
	VT_DESERIALIZE_PROPERTY(regionMergeSize, s_editorSettings.navmeshBuildSettings.regionMergeSize, navMeshBuildnode, 20.f);
	VT_DESERIALIZE_PROPERTY(edgeMaxLen, s_editorSettings.navmeshBuildSettings.edgeMaxLen, navMeshBuildnode, 12.f);
	VT_DESERIALIZE_PROPERTY(edgeMaxError, s_editorSettings.navmeshBuildSettings.edgeMaxError, navMeshBuildnode, 1.3f);
	VT_DESERIALIZE_PROPERTY(vertsPerPoly, s_editorSettings.navmeshBuildSettings.vertsPerPoly, navMeshBuildnode, 6.f);
	VT_DESERIALIZE_PROPERTY(detailSampleDist, s_editorSettings.navmeshBuildSettings.detailSampleDist, navMeshBuildnode, 6.f);
	VT_DESERIALIZE_PROPERTY(detailSampleMaxError, s_editorSettings.navmeshBuildSettings.detailSampleMaxError, navMeshBuildnode, 1.f);
	VT_DESERIALIZE_PROPERTY(partitionType, s_editorSettings.navmeshBuildSettings.partitionType, navMeshBuildnode, 0);
	VT_DESERIALIZE_PROPERTY(useTileCache, s_editorSettings.navmeshBuildSettings.useTileCache, navMeshBuildnode, false);
	VT_DESERIALIZE_PROPERTY(useAutoBaking, s_editorSettings.navmeshBuildSettings.useAutoBaking, navMeshBuildnode, true);

	YAML::Node Networknode = settingsNode["NetworkSettings"];
	VT_DESERIALIZE_PROPERTY(enableNetworking, s_editorSettings.networkSettings.enableNetworking, Networknode, false);
}

void UserSettingsManager::SaveUserSettings()
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Settings" << YAML::Value;
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Windows" << YAML::BeginSeq;
		for (const auto& window : EditorLibrary::GetPanels())
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY_STRING(window.editorWindow->GetTitle(), window.editorWindow->IsOpen(), out);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "SceneSettings" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(worldSpace, s_editorSettings.sceneSettings.worldSpace, out);
			VT_SERIALIZE_PROPERTY(snapToGrid, s_editorSettings.sceneSettings.snapToGrid, out);
			VT_SERIALIZE_PROPERTY(snapRotation, s_editorSettings.sceneSettings.snapRotation, out);
			VT_SERIALIZE_PROPERTY(snapScale, s_editorSettings.sceneSettings.snapScale, out);
			VT_SERIALIZE_PROPERTY(showGizmos, s_editorSettings.sceneSettings.showGizmos, out);
			VT_SERIALIZE_PROPERTY(use16by9, s_editorSettings.sceneSettings.use16by9, out);
			VT_SERIALIZE_PROPERTY(fullscreenOnPlay, s_editorSettings.sceneSettings.fullscreenOnPlay, out);
			VT_SERIALIZE_PROPERTY(gridEnabled, s_editorSettings.sceneSettings.gridEnabled, out);
			VT_SERIALIZE_PROPERTY(gridSnapValue, s_editorSettings.sceneSettings.gridSnapValue, out);
			VT_SERIALIZE_PROPERTY(rotationSnapValue, s_editorSettings.sceneSettings.rotationSnapValue, out);
			VT_SERIALIZE_PROPERTY(scaleSnapValue, s_editorSettings.sceneSettings.scaleSnapValue, out);
			VT_SERIALIZE_PROPERTY(lastOpenScene, s_editorSettings.sceneSettings.lastOpenScene.string(), out);

			VT_SERIALIZE_PROPERTY(showLightSpheres, s_editorSettings.sceneSettings.showLightSpheres, out);
			VT_SERIALIZE_PROPERTY(showEntityGizmos, s_editorSettings.sceneSettings.showEntityGizmos, out);
			VT_SERIALIZE_PROPERTY(showBoundingSpheres, s_editorSettings.sceneSettings.showBoundingSpheres, out);
			VT_SERIALIZE_PROPERTY(colliderViewMode, (uint32_t)s_editorSettings.sceneSettings.colliderViewMode, out);
			VT_SERIALIZE_PROPERTY(showEnvironmentProbes, s_editorSettings.sceneSettings.showEnvironmentProbes, out);
			VT_SERIALIZE_PROPERTY(navMeshViewMode, (uint32_t)s_editorSettings.sceneSettings.navMeshViewMode, out);
			VT_SERIALIZE_PROPERTY(lowMemoryUsage, s_editorSettings.sceneSettings.lowMemoryUsage, out);
			out << YAML::EndMap;
		}

		out << YAML::Key << "VersionControlSettings" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(server, s_editorSettings.versionControlSettings.server, out);
			VT_SERIALIZE_PROPERTY(user, s_editorSettings.versionControlSettings.user, out);
			VT_SERIALIZE_PROPERTY(password, s_editorSettings.versionControlSettings.password, out);
			VT_SERIALIZE_PROPERTY(workspace, s_editorSettings.versionControlSettings.workspace, out);
			VT_SERIALIZE_PROPERTY(stream, s_editorSettings.versionControlSettings.stream, out);
			out << YAML::EndMap;
		}

		out << YAML::Key << "ExternalToolsSettings" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(customExternalScriptEditor, s_editorSettings.externalToolsSettings.customExternalScriptEditor.string(), out);
			out << YAML::EndMap;
		}

		out << YAML::Key << "NavMeshBuildSettings" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(maxAgents, s_editorSettings.navmeshBuildSettings.maxAgents, out);
			VT_SERIALIZE_PROPERTY(agentHeight, s_editorSettings.navmeshBuildSettings.agentHeight, out);
			VT_SERIALIZE_PROPERTY(agentRadius, s_editorSettings.navmeshBuildSettings.agentRadius, out);
			VT_SERIALIZE_PROPERTY(agentMaxClimb, s_editorSettings.navmeshBuildSettings.agentMaxClimb, out);
			VT_SERIALIZE_PROPERTY(agentMaxSlope, s_editorSettings.navmeshBuildSettings.agentMaxSlope, out);

			VT_SERIALIZE_PROPERTY(cellSize, s_editorSettings.navmeshBuildSettings.cellSize, out);
			VT_SERIALIZE_PROPERTY(cellHeight, s_editorSettings.navmeshBuildSettings.cellHeight, out);
			VT_SERIALIZE_PROPERTY(regionMinSize, s_editorSettings.navmeshBuildSettings.regionMinSize, out);
			VT_SERIALIZE_PROPERTY(regionMergeSize, s_editorSettings.navmeshBuildSettings.regionMergeSize, out);
			VT_SERIALIZE_PROPERTY(edgeMaxLen, s_editorSettings.navmeshBuildSettings.edgeMaxLen, out);
			VT_SERIALIZE_PROPERTY(edgeMaxError, s_editorSettings.navmeshBuildSettings.edgeMaxError, out);
			VT_SERIALIZE_PROPERTY(vertsPerPoly, s_editorSettings.navmeshBuildSettings.vertsPerPoly, out);
			VT_SERIALIZE_PROPERTY(detailSampleDist, s_editorSettings.navmeshBuildSettings.detailSampleDist, out);
			VT_SERIALIZE_PROPERTY(detailSampleMaxError, s_editorSettings.navmeshBuildSettings.detailSampleMaxError, out);
			VT_SERIALIZE_PROPERTY(partitionType, s_editorSettings.navmeshBuildSettings.partitionType, out);
			VT_SERIALIZE_PROPERTY(useTileCache, s_editorSettings.navmeshBuildSettings.useTileCache, out);
			VT_SERIALIZE_PROPERTY(useAutoBaking, s_editorSettings.navmeshBuildSettings.useAutoBaking, out);
			out << YAML::EndMap;
		}

		out << YAML::Key << "NetworkSettings" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(enableNetworking, s_editorSettings.networkSettings.enableNetworking, out);
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}
	out << YAML::EndMap;

	std::ofstream fout(s_userSettingsPath);
	fout << out.c_str();
	fout.close();
}
