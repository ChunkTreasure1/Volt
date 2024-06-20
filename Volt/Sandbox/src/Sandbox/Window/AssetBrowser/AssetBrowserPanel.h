#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetBrowser/AssetCommon.h"

#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/KeyEvent.h>
#include <Volt/Events/MouseEvent.h>

#include <glm/glm.hpp>

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

class AssetPreview;
class PreviewRenderer;
class AssetBrowserPanel : public EditorWindow
{
public:
	AssetBrowserPanel(Ref<Volt::Scene>& aScene, const std::string& id);

	void UpdateMainContent() override;
	void OnEvent(Volt::Event& e) override;

	void Reload();

private:
	bool OnDragDropEvent(Volt::WindowDragDropEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnMouseReleasedEvent(Volt::MouseButtonReleasedEvent& e);
	bool OnRenderEvent(Volt::AppRenderEvent& e);

	std::vector<AssetBrowser::DirectoryItem*> FindParentDirectoriesOfDirectory(AssetBrowser::DirectoryItem* directory);

	void RenderControlsBar(float height);
	bool RenderDirectory(const Ref<AssetBrowser::DirectoryItem> dirData);
	void RenderView(std::vector<Ref<AssetBrowser::DirectoryItem>>& directories, std::vector<Ref<AssetBrowser::AssetItem>>& assets);
	void RenderWindowRightClickPopup();

	void DeleteFilesModal();

	void Search(const std::string& query);
	void FindFoldersAndFilesWithQuery(const std::vector<Ref<AssetBrowser::DirectoryItem>>& dirList, std::vector<Ref<AssetBrowser::DirectoryItem>>& directories, std::vector<Ref<AssetBrowser::AssetItem>>& assets, const std::string& query);

	AssetBrowser::DirectoryItem* FindDirectoryWithPath(const std::filesystem::path& path);
	AssetBrowser::DirectoryItem* FindDirectoryWithPathRecursivly(const std::vector<Ref<AssetBrowser::DirectoryItem>> dirList, const std::filesystem::path& path);

	void CreatePrefabAndSetupEntities(Volt::EntityID entity);
	void SetupEntityAsPrefab(Volt::EntityID entity, Volt::AssetHandle prefabId);

	void RecursiveRemoveFolderContents(DirectoryData* aDir);
	void RecursiceRenameFolderContents(DirectoryData* aDir, const std::filesystem::path& newDir);

	void ClearAssetPreviewsInCurrentDirectory();

	float GetThumbnailSize();

	///// Asset Creation /////	
	void CreateNewAssetInCurrentDirectory(Volt::AssetType type);
	void CreateNewShaderModal();
	void CreateNewMonoScriptModal();
	void CreateNewMotionWeaveDatabaseModal();

	struct NewShaderData
	{
		std::string name = "New Shader";
		bool createPixelShader = true;
		bool createGeometryShader = false;
		bool createVertexShader = false;

		int32_t shaderType = 0;

	} myNewShaderData;

	struct NewMotionWeaveDatabaseData
	{
		std::string name = "New Motion Weave Database";
		Volt::AssetHandle skeleton = Volt::Asset::Null();
	} m_NewMotionWeaveDatabaseData;
	//////////////////////////

	Ref<Volt::Scene>& myEditorScene;
	Ref<PreviewRenderer> myPreviewRenderer;

	std::vector<AssetBrowser::DirectoryItem*> myDirectoryButtons;

	AssetBrowser::DirectoryItem* myAssetsDirectory = nullptr;

	float myThumbnailPadding = 16.f;
	bool myHasSearchQuery = false;
	bool myShouldDeleteSelected = false;

	glm::vec2 myViewBounds[2];

	std::string mySearchQuery;
	std::vector<Ref<AssetBrowser::DirectoryItem>> mySearchDirectories;
	std::vector<Ref<AssetBrowser::AssetItem>> mySearchAssets;

	///// Mesh import data //////
	AssetData myMeshToImport;
	MeshImportData myMeshImportData;
	Volt::AssetType myAssetMask = Volt::AssetType::None;
	
	std::vector<std::filesystem::path> myDragDroppedMeshes;
	std::vector<std::filesystem::path> myDragDroppedTextures;

	bool myIsImporting = false;

	Volt::AssetHandle myAnimationReimportTargetSkeleton;

	///// Animated Character creation /////
	NewCharacterData myNewCharacterData{};
	Ref<Volt::AnimatedCharacter> myNewAnimatedCharacter;

	///// Animation Graph creation /////
	NewAnimationGraphData myNewAnimationGraphData{};

	std::unordered_map <std::filesystem::path, Ref<AssetBrowser::DirectoryItem>> myDirectories;
	Ref<AssetBrowser::SelectionManager> mySelectionManager;

	AssetBrowser::DirectoryItem* myCurrentDirectory = nullptr;
	AssetBrowser::DirectoryItem* myNextDirectory = nullptr;
};
