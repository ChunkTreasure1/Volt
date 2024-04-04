#pragma once

#include "Sandbox/Window/AssetBrowser/AssetCommon.h"
#include "Sandbox/Utility/AssetBrowserPopup.h"

#include <Volt/Asset/Asset.h>
#include <Volt/Utility/UIUtility.h>

#include <filesystem>
#include <format>

namespace Volt
{
	class AnimationGraphAsset;
	class Mesh;
}

struct MeshImportData
{
	std::filesystem::path destination;
	Volt::AssetHandle externalMaterial;
	Volt::AssetHandle targetSkeleton;
	bool createMaterial = true;
	bool importMesh = true;
	bool importSkeleton = false;
	bool importAnimation = false;
};

struct NewCharacterData
{
	std::string name = "None";
	Volt::AssetHandle skeletonHandle = Volt::Asset::Null();
	Volt::AssetHandle skinHandle = Volt::Asset::Null();
	std::filesystem::path destination = "Assets/Animations/";
};

struct NewAnimationGraphData
{
	std::string name = "None";
	Volt::AssetHandle skeletonHandle = Volt::Asset::Null();

	std::filesystem::path destination = "Assets/Animations/";
};

enum class SaveReturnState
{
	None,
	Save,
	Discard
};

enum class ImportState
{
	None,
	Imported,
	Discard
};

class EditorUtils
{
public:
	static bool Property(const std::string& text, Volt::AssetHandle& assetHandle, Volt::AssetType wantedType = Volt::AssetType::None);
	static bool AssetBrowserPopupField(const std::string& id, Volt::AssetHandle& assetHandle, Volt::AssetType wantedType = Volt::AssetType::None);

	static bool SearchBar(std::string& outSearchQuery, bool& outHasSearchQuery, bool setAsActive = false);

	static bool ReimportSourceMesh(Volt::AssetHandle assetHandle, Ref<Volt::Skeleton> targetSkeleton = nullptr);

	static ImportState MeshImportModal(const std::string& aId, MeshImportData& aImportData, const std::filesystem::path& aMeshToImport);
	static ImportState MeshBatchImportModal(const std::string& aId, MeshImportData& aImportData, const std::vector<std::filesystem::path>& meshesToImport);

	static void MeshExportModal(const std::string& aId, std::filesystem::path aDirectoryPath, MeshImportData& aExportData, std::vector<Ref<Volt::Mesh>> aMeshesToExport);
	static void ImportTexture(const std::filesystem::path& sourcePath);

	static bool NewCharacterModal(const std::string& aId, Ref<Volt::AnimatedCharacter>& outCharacter, NewCharacterData& aCharacterData);
	static bool NewAnimationGraphModal(const std::string& aId, Ref<Volt::AnimationGraphAsset>* outGraph, NewAnimationGraphData& graphData);

	static SaveReturnState SaveFilePopup(const std::string& aId);

	static Ref<Volt::Texture2D> GenerateThumbnail(const std::filesystem::path& path);
	static bool HasThumbnail(const std::filesystem::path& path);
	static std::filesystem::path GetThumbnailPathFromPath(const std::filesystem::path& path);

	static void MarkEntityAsEdited(const Volt::Entity& entity);
	static void MarkEntityAndChildrenAsEdited(const Volt::Entity& entity);

private:
	static bool AssetBrowserPopupInternal(const std::string& id, Volt::AssetHandle& assetHandle, bool startState, Volt::AssetType wantedType = Volt::AssetType::None);
	struct DefaultFalse
	{
		bool state = false;
	};

	inline static std::unordered_map<std::string, Ref<AssetBrowserPopup>> s_assetBrowserPopups;
	inline static std::unordered_map<std::string, DefaultFalse> s_assetBrowserPopupsOpen;

	EditorUtils() = delete;
};
