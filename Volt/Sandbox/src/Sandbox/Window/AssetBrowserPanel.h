#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetCommon.h"

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

	Ref<DirectoryData> ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent);
	std::vector<DirectoryData*> FindParentDirectoriesOfDirectory(DirectoryData* directory);

	void RenderControlsBar(float height);
	void RenderDirectory(const Ref<DirectoryData> dirData);
	void RenderView(std::vector<Ref<DirectoryData>>& dirData, std::vector<AssetData>& assetData);
	bool RenderFilePopup(AssetData& data);
	bool RenderFolderPopup(DirectoryData* data);
	void RenderFileInfo(const AssetData& data);

	void DeleteFilesModal();

	void Reload();
	void RenderDirectories(std::vector<Ref<DirectoryData>>& dirData);
	void RenderAssets(std::vector<AssetData>& assetData);

	void RenderShaderPopup(const AssetData& data);
	void RenderMeshSourcePopup(const AssetData& data);
	void RenderMeshPopup(const AssetData& data);
	void RenderAnimationPopup(const AssetData& data);
	void RenderSkeletonPopup(const AssetData& data);

	void Search(const std::string& query);
	void FindFoldersAndFilesWithQuery(const std::vector<Ref<DirectoryData>>& dirList, std::vector<Ref<DirectoryData>>& directories, std::vector<AssetData>& assets, const std::string& query);

	DirectoryData* FindDirectoryWithPath(const std::filesystem::path& path);
	DirectoryData* FindDirectoryWithPathRecursivly(const std::vector<Ref<DirectoryData>> dirList, const std::filesystem::path& path);

	void CreatePrefabAndSetupEntities(Wire::EntityId entity);
	void SetupEntityAsPrefab(Wire::EntityId entity, Volt::AssetHandle prefabId);

	void DeselectAllDirectories(DirectoryData* rootDir);
	void DeselectAllAssets(DirectoryData* rootDir);

	void RecursiveRemoveFolderContents(DirectoryData* aDir);
	void RecursiceRenameFolderContents(DirectoryData* aDir, const std::filesystem::path& newDir);

	void GenerateAssetPreviewsInCurrentDirectory();
	void CreateNewAssetInCurrentDirectory(Volt::AssetType type);

	Ref<Volt::Scene>& myEditorScene;

	std::unordered_map<std::string, Ref<DirectoryData>> myDirectories;
	std::unordered_map<Volt::AssetHandle, Ref<AssetPreview>> myAssetPreviews;

	std::vector<Ref<AssetPreview>> myPreviewsToUpdate;
	std::vector<DirectoryData*> myDirectoryButtons;

	DirectoryData* myCurrentDirectory = nullptr;
	DirectoryData* myNextDirectory = nullptr;

	DirectoryData* myEngineDirectory = nullptr;
	DirectoryData* myAssetsDirectory = nullptr;

	float myThumbnailPadding = 16.f;
	float myThumbnailSize = 85.f;
	bool myShowEngineAssets = false;
	bool myHasSearchQuery = false;
	bool myShouldDeleteSelected = false;
	
	Volt::AssetHandle myCurrentlyRenamingAsset = Volt::Asset::Null();
	Volt::AssetHandle myLastRenamingAsset = Volt::Asset::Null();
	std::string myCurrentlyRenamingAssetName;

	DirectoryData* myCurrentlyRenamingDirectory = nullptr;
	DirectoryData* myLastRenamingDirectory = nullptr;
	std::string myCurrentlyRenamingDirectoryName;

	gem::vec2 myViewBounds[2];

	std::string mySearchQuery;
	std::vector<Ref<DirectoryData>> mySearchDirectories;
	std::vector<AssetData> mySearchAssets;

	std::unordered_map<Volt::AssetType, Ref<Volt::Texture2D>> myAssetIcons;

	///// Mesh import data //////
	AssetData myMeshToImport;
	MeshImportData myMeshImportData;
	Volt::AssetType myAssetMask = Volt::AssetType::None;
	std::vector<std::filesystem::path> myDragDroppedMeshes;
	bool myIsImporting = false;

	///// Animated Character creation /////
	NewCharacterData myNewCharacterData{};
	Ref<Volt::AnimatedCharacter> myNewAnimatedCharacter;
};