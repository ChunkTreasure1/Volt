#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetBrowser/AssetCommon.h"

#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/KeyEvent.h>
#include <Volt/Events/MouseEvent.h>

#include <gem/gem.h>
#include <Wire/Wire.h>

namespace Volt
{
	class Texture2D;
	class Scene;
}

namespace AssetBrowser
{
	class DirectoryItem;
	class AssetItem;
	class SelectionManager;
}

class AssetBrowserPanel : public EditorWindow
{
public:
	AssetBrowserPanel(Ref<Volt::Scene>& aScene);

	void UpdateMainContent() override;
	void OnEvent(Volt::Event& e) override;

private:
	bool OnDragDropEvent(Volt::WindowDragDropEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnMouseReleasedEvent(Volt::MouseButtonReleasedEvent& e);
	bool OnRenderEvent(Volt::AppRenderEvent& e);

	std::vector<AssetBrowser::DirectoryItem*> FindParentDirectoriesOfDirectory(AssetBrowser::DirectoryItem* directory);

	void RenderControlsBar(float height);
	bool RenderDirectory(const Ref<AssetBrowser::DirectoryItem> dirData);
	void RenderView(std::vector<Ref<AssetBrowser::DirectoryItem>>& directories, std::vector<Ref<AssetBrowser::AssetItem>>& assets);
	void RenderFileInfo(const AssetData& data);
	void RenderWindowRightClickPopup();

	void DeleteFilesModal();

	void Reload();

	void Search(const std::string& query);
	void FindFoldersAndFilesWithQuery(const std::vector<Ref<AssetBrowser::DirectoryItem>>& dirList, std::vector<Ref<AssetBrowser::DirectoryItem>>& directories, std::vector<Ref<AssetBrowser::AssetItem>>& assets, const std::string& query);

	AssetBrowser::DirectoryItem* FindDirectoryWithPath(const std::filesystem::path& path);
	AssetBrowser::DirectoryItem* FindDirectoryWithPathRecursivly(const std::vector<Ref<AssetBrowser::DirectoryItem>> dirList, const std::filesystem::path& path);

	void CreatePrefabAndSetupEntities(Wire::EntityId entity);
	void SetupEntityAsPrefab(Wire::EntityId entity, Volt::AssetHandle prefabId);

	void RecursiveRemoveFolderContents(DirectoryData* aDir);
	void RecursiceRenameFolderContents(DirectoryData* aDir, const std::filesystem::path& newDir);

	void GenerateAssetPreviewsInCurrentDirectory();
	void CreateNewAssetInCurrentDirectory(Volt::AssetType type);

	Ref<Volt::Scene>& myEditorScene;

	std::unordered_map<Volt::AssetHandle, Ref<AssetPreview>> myAssetPreviews;

	std::vector<Ref<AssetPreview>> myPreviewsToUpdate;
	std::vector<AssetBrowser::DirectoryItem*> myDirectoryButtons;

	AssetBrowser::DirectoryItem* myEngineDirectory = nullptr;
	AssetBrowser::DirectoryItem* myAssetsDirectory = nullptr;

	float myThumbnailPadding = 16.f;
	float myThumbnailSize = 85.f;
	bool myShowEngineAssets = false;
	bool myHasSearchQuery = false;
	bool myShouldDeleteSelected = false;

	gem::vec2 myViewBounds[2];

	std::string mySearchQuery;
	std::vector<Ref<AssetBrowser::DirectoryItem>> mySearchDirectories;
	std::vector<Ref<AssetBrowser::AssetItem>> mySearchAssets;

	///// Mesh import data //////
	AssetData myMeshToImport;
	MeshImportData myMeshImportData;
	Volt::AssetType myAssetMask = Volt::AssetType::None;
	std::vector<std::filesystem::path> myDragDroppedMeshes;
	bool myIsImporting = false;

	///// Animated Character creation /////
	NewCharacterData myNewCharacterData{};
	Ref<Volt::AnimatedCharacter> myNewAnimatedCharacter;


	Ref<AssetBrowser::DirectoryItem> ProcessDirectory(const std::filesystem::path& path, AssetBrowser::DirectoryItem* parent);
	std::unordered_map <std::filesystem::path, Ref<AssetBrowser::DirectoryItem>> myDirectories;
	Ref<AssetBrowser::SelectionManager> mySelectionManager;

	AssetBrowser::DirectoryItem* myCurrentDirectory = nullptr;
	AssetBrowser::DirectoryItem* myNextDirectory = nullptr;
};