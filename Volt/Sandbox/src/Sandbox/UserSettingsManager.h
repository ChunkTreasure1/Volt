#pragma once

#include "Sandbox/VersionControl/VersionControl.h"

struct WindowSettings
{
	std::unordered_map<std::string, bool> windowsOpen;
};

struct SceneSettings
{
	bool worldSpace = true;
	bool snapToGrid = false;
	bool snapRotation = false;
	bool snapScale = false;
	bool showGizmos = false;
	bool use16by9 = false;
	bool fullscreenOnPlay = false;

	float gridSnapValue;
	float rotationSnapValue;
	float scaleSnapValue;

	std::filesystem::path lastOpenScene;
};

struct EditorSettings
{
	WindowSettings windowSettings;
	SceneSettings sceneSettings;
	VersionControlSettings versionControlSettings;
};

class EditorWindow;
class UserSettingsManager
{
public:
	static void LoadUserSettings(const std::vector<Ref<EditorWindow>>& windows);
	static void SaveUserSettings(const std::vector<Ref<EditorWindow>>& windows);

	inline static EditorSettings& GetSettings() { return s_editorSettings; }

private:
	inline static EditorSettings s_editorSettings;

	UserSettingsManager() = delete;
};
