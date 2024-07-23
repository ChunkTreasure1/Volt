#pragma once

#include "Sandbox/VersionControl/VersionControl.h"

#include <NavigationEditor/Builder/BuildInterfaces.h>

struct WindowSettings
{
	std::unordered_map<std::string, bool> windowsOpen;
};

enum class NavMeshViewMode : uint32_t
{
	None = 0,
	All,
	Only,
};

enum class ColliderViewMode : uint32_t
{
	None = 0,
	Selected,
	All,
	AllHideMesh
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
	bool gridEnabled = false;

	float gridSnapValue = 0.f;
	float rotationSnapValue = 0.f;
	float scaleSnapValue = 0.f;

	bool showLightSpheres = true;
	bool showEntityGizmos = true;
	bool showBoundingSpheres = false;
	ColliderViewMode colliderViewMode = ColliderViewMode::None;
	bool showEnvironmentProbes = false;
	NavMeshViewMode navMeshViewMode = NavMeshViewMode::None;

	std::filesystem::path lastOpenScene;
};

struct ExternalToolsSettings
{
	std::unordered_map<std::string, std::filesystem::path> scriptEditorPaths;
	std::filesystem::path customExternalScriptEditor;
};

struct NetworkSettings
{
	bool enableNetworking = false;
};

struct AssetBrowserSettings
{
	float thumbnailSize = 85.f;
};

struct PanelState
{
	std::string panelName;
	bool isOpen = false;
};

struct EditorSettings
{
	WindowSettings windowSettings;
	SceneSettings sceneSettings;
	VersionControlSettings versionControlSettings;
	ExternalToolsSettings externalToolsSettings;
	RecastBuildSettings navmeshBuildSettings;
	NetworkSettings networkSettings;
	AssetBrowserSettings assetBrowserSettings;

	Vector<PanelState> panelStates;
};

class EditorWindow;
class UserSettingsManager
{
public:
	static void LoadUserSettings();
	static void SaveUserSettings();

	static void SetupPanels();

	inline static EditorSettings& GetSettings() { return s_editorSettings; }

private:
	inline static EditorSettings s_editorSettings;

	UserSettingsManager() = delete;
};
