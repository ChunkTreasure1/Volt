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

	AssetBrowserPopup(const std::string& id, Volt::AssetType wantedType, Volt::AssetHandle& handler);

	State Update();

private:
	State RenderView(const Vector<Volt::AssetHandle>& items);

	std::string myId;
	Volt::AssetType myWantedType;
	Volt::AssetHandle& myHandle;

	std::string mySearchQuery;
	bool myActivateSearch = false;
};
