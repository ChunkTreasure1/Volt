#pragma once

#include <Volt/Asset/Asset.h>

struct GlobalEditorStates
{
	inline static bool isDragging = false;
	inline static bool dragStartedInAssetBrowser = false;
	inline static Volt::AssetHandle dragAsset = Volt::Asset::Null();
};