#pragma once

#include <Volt/Asset/Asset.h>

class AssetBrowserPopup
{
public:
	enum class State
	{
		Changed,
		Closed,
		Open
	};

	AssetBrowserPopup(const std::string& id, Volt::AssetType wantedType, Volt::AssetHandle& handle, std::function<void(Volt::AssetHandle& value)> callback = nullptr);

	State Update();

private:
	State RenderView(const std::vector<std::filesystem::path>& items);

	std::string myId;
	Volt::AssetType myWantedType;
	Volt::AssetHandle& myHandle;
	std::function<void(Volt::AssetHandle& value)> myCallback;

	std::string mySearchQuery;
	bool myActivateSearch = false;
};