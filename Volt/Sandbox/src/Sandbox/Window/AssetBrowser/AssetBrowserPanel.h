#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetBrowser/AssetCommon.h"

#include "Sandbox/Utility/EditorUtilities.h"

#include <glm/glm.hpp>

namespace Volt
{
	class Texture2D;
	class Scene;

	class WindowDragDropEvent;
	class KeyPressedEvent;
	class MouseButtonReleasedEvent;
	class WindowRenderEvent;
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
	void Reload();

private:
	bool OnDragDropEvent(Volt::WindowDragDropEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnMouseReleasedEvent(Volt::MouseButtonReleasedEvent& e);
	bool OnRenderEvent(Volt::WindowRenderEvent& e);

	Vector<AssetBrowser::DirectoryItem*> FindParentDirectoriesOfDirectory(AssetBrowser::DirectoryItem* directory);

	void RenderControlsBar(float height);
	bool RenderDirectory(const Ref<AssetBrowser::DirectoryItem> dirData);
	void RenderView(Vector<Ref<AssetBrowser::DirectoryItem>>& directories, Vector<Ref<AssetBrowser::AssetItem>>& assets);
	void RenderWindowRightClickPopup();

	void DeleteFilesModal();

	void Search(const std::string& query);
	void FindFoldersAndFilesWithQuery(const Vector<Ref<AssetBrowser::DirectoryItem>>& dirList, Vector<Ref<AssetBrowser::DirectoryItem>>& directories, Vector<Ref<AssetBrowser::AssetItem>>& assets, const std::string& query);

	AssetBrowser::DirectoryItem* FindDirectoryWithPath(const std::filesystem::path& path);
	AssetBrowser::DirectoryItem* FindDirectoryWithPathRecursivly(const Vector<Ref<AssetBrowser::DirectoryItem>> dirList, const std::filesystem::path& path);

	void CreatePrefabAndSetupEntities(Volt::EntityID entity);
	void SetupEntityAsPrefab(Volt::EntityID entity, Volt::AssetHandle prefabId);

	void RecursiveRemoveFolderContents(DirectoryData* aDir);
	void RecursiceRenameFolderContents(DirectoryData* aDir, const std::filesystem::path& newDir);

	void ClearAssetPreviewsInCurrentDirectory();

	float GetThumbnailSize();

	///// Asset Creation /////	
	void CreateNewAssetInCurrentDirectory(AssetType type);
	void CreateNewShaderModal();
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

	Vector<AssetBrowser::DirectoryItem*> myDirectoryButtons;

	AssetBrowser::DirectoryItem* myAssetsDirectory = nullptr;

	float myThumbnailPadding = 16.f;
	bool myHasSearchQuery = false;
	bool myShouldDeleteSelected = false;

	glm::vec2 myViewBounds[2];

	std::string mySearchQuery;
	Vector<Ref<AssetBrowser::DirectoryItem>> mySearchDirectories;
	Vector<Ref<AssetBrowser::AssetItem>> mySearchAssets;

	///// Mesh import data //////
	AssetData myMeshToImport;
	MeshImportData myMeshImportData;
	std::set<AssetType> m_assetMask;
	
	Vector<std::filesystem::path> myDragDroppedMeshes;
	Vector<std::filesystem::path> myDragDroppedTextures;

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
