#pragma once

#include "Sandbox/Utility/AssetPreview.h"

#include <AssetSystem/Asset.h>

struct AssetData
{
	Volt::AssetHandle handle = 0;
	AssetType type = AssetTypes::None;
	std::filesystem::path path;
	bool selected = false;
};

struct DirectoryData
{
	Volt::AssetHandle handle;
	std::filesystem::path path;

	DirectoryData* parentDir;
	bool selected = false;

	Vector<AssetData> assets;
	Vector<Ref<DirectoryData>> subDirectories;
};
