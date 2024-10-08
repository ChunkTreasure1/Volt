#pragma once

#include "Sandbox/Utility/AssetBrowserPopup.h"

#include <AssetSystem/Asset.h>

#include <filesystem>

namespace Volt
{
	class AnimationGraphAsset;
	class AnimatedCharacter;
	class Texture2D;
	class Mesh;
	class Entity;
}

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

class EditorUtils
{
public:
	static bool Property(const std::string& text, Volt::AssetHandle& assetHandle, AssetType wantedType = AssetTypes::None);
	static bool AssetBrowserPopupField(const std::string& id, Volt::AssetHandle& assetHandle, AssetType wantedType = AssetTypes::None);

	static bool SearchBar(std::string& outSearchQuery, bool& outHasSearchQuery, bool setAsActive = false);

	static bool NewCharacterModal(const std::string& aId, Ref<Volt::AnimatedCharacter>& outCharacter, NewCharacterData& aCharacterData);

	static SaveReturnState SaveFilePopup(const std::string& aId);

	static Ref<Volt::Texture2D> GenerateThumbnail(const std::filesystem::path& path);
	static bool HasThumbnail(const std::filesystem::path& path);
	static std::filesystem::path GetThumbnailPathFromPath(const std::filesystem::path& path);

	static std::string GetDuplicatedNameFromEntity(const Volt::Entity& entity);

	static void MarkEntityAsEdited(const Volt::Entity& entity);
	static void MarkEntityAndChildrenAsEdited(const Volt::Entity& entity);

private:
	static bool AssetBrowserPopupInternal(const std::string& id, Volt::AssetHandle& assetHandle, bool startState, AssetType wantedType = AssetTypes::None);
	struct DefaultFalse
	{
		bool state = false;
	};

	inline static std::unordered_map<std::string, Ref<AssetBrowserPopup>> s_assetBrowserPopups;
	inline static std::unordered_map<std::string, DefaultFalse> s_assetBrowserPopupsOpen;

	EditorUtils() = delete;
};
