#include "sbpch.h"
#include "UserSettingsManager.h"

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Utility/YAMLSerializationHelpers.h>
#include <Volt/Utility/SerializationMacros.h>

#include <yaml-cpp/yaml.h>

inline static const std::filesystem::path s_userSettingsPath = "User/UserSettings.yaml";

void UserSettingsManager::LoadUserSettings(const std::vector<Ref<EditorWindow>>& windows)
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
	for (const auto& window : windows)
	{
		if (windowsNode[window->GetTitle()])
		{
			bool value = windowsNode[window->GetTitle()].as<bool>();

			if (value)
			{
				window->Open();
			}
			else
			{
				window->Close();
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
	VT_DESERIALIZE_PROPERTY(gridSnapValue, s_editorSettings.sceneSettings.gridSnapValue, sceneSettingsNode, 50.f);
	VT_DESERIALIZE_PROPERTY(rotationSnapValue, s_editorSettings.sceneSettings.rotationSnapValue, sceneSettingsNode, 45.f);
	VT_DESERIALIZE_PROPERTY(scaleSnapValue, s_editorSettings.sceneSettings.scaleSnapValue, sceneSettingsNode, 0.1f);

	std::string sceneInput;
	VT_DESERIALIZE_PROPERTY(lastOpenScene, sceneInput, sceneSettingsNode, std::string());
	s_editorSettings.sceneSettings.lastOpenScene = sceneInput;

	YAML::Node versionControlnode = settingsNode["VersionControlSettings"];
	VT_DESERIALIZE_PROPERTY(server, s_editorSettings.versionControlSettings.server, versionControlnode, std::string("localhost:1666"));
	VT_DESERIALIZE_PROPERTY(user, s_editorSettings.versionControlSettings.user, versionControlnode, std::string());
	VT_DESERIALIZE_PROPERTY(password, s_editorSettings.versionControlSettings.password, versionControlnode, std::string());
}

void UserSettingsManager::SaveUserSettings(const std::vector<Ref<EditorWindow>>& windows)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Settings" << YAML::Value;
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Windows" << YAML::BeginSeq;
		for (const auto& window : windows)
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY_STRING(window->GetTitle(), window->IsOpen(), out);
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
			VT_SERIALIZE_PROPERTY(gridSnapValue, s_editorSettings.sceneSettings.gridSnapValue, out);
			VT_SERIALIZE_PROPERTY(rotationSnapValue, s_editorSettings.sceneSettings.rotationSnapValue, out);
			VT_SERIALIZE_PROPERTY(scaleSnapValue, s_editorSettings.sceneSettings.scaleSnapValue, out);
			VT_SERIALIZE_PROPERTY(lastOpenScene, s_editorSettings.sceneSettings.lastOpenScene.string(), out);
			out << YAML::EndMap;
		}

		out << YAML::Key << "VersionControlSettings" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(server, s_editorSettings.versionControlSettings.server, out);
			VT_SERIALIZE_PROPERTY(user, s_editorSettings.versionControlSettings.user, out);
			VT_SERIALIZE_PROPERTY(password, s_editorSettings.versionControlSettings.password, out);
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}
	out << YAML::EndMap;

	std::ofstream fout(s_userSettingsPath);
	fout << out.c_str();
	fout.close();
}
