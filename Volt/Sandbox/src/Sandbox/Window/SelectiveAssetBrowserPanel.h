#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Window/AssetCommon.h"

class SelectiveAssetBrowserPanel : public EditorWindow
{
public:
	SelectiveAssetBrowserPanel(Volt::AssetType assetType, const std::string& id);

	void UpdateMainContent() override;
	inline void SetOpenFileCallback(std::function<void(Volt::AssetHandle asset)>&& function) { myOpenFileCallback = function; }

private:
	void UpdateAssetList();
	void RenderControlsBar();

	Volt::AssetType mySelectiveAssetType;

	std::vector<AssetData> myAllAssetsOfType;
	std::function<void(Volt::AssetHandle asset)> myOpenFileCallback;

	float myThumbnailSize = 70.f;
	float myThumbnailPadding = 16.f;
	const float myControlsBarHeight = 30.f;
};