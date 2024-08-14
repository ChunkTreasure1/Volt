#pragma once

#include <AssetSystem/Asset.h>

#include <CoreUtilities/Containers/Vector.h>

class AssetBrowserPopup
{
public:
	enum class State
	{
		Changed,
		Closed,
		Open
	};

	AssetBrowserPopup(const std::string& id, AssetType wantedType, Volt::AssetHandle& handler);

	State Update();

private:
	State RenderView(const Vector<Volt::AssetHandle>& items);

	std::string myId;
	AssetType myWantedType;
	Volt::AssetHandle& myHandle;

	std::string mySearchQuery;
	bool myActivateSearch = false;
};
