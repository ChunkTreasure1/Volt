#pragma once

#include <AssetSystem/Asset.h>
#include <CoreUtilities/Containers/Vector.h>

#include <functional>

typedef std::function<Vector<std::pair<std::string, std::string>>(Volt::AssetHandle)> AssetBrowserPopupDataFunction;
//a class for registring data and open function for the asset browser
struct EditorAssetData
{
	EditorAssetData(AssetBrowserPopupDataFunction aAssetBrowserPopupDataFunction = nullptr)
		: assetBrowserPopupDataFunction(aAssetBrowserPopupDataFunction)
	{
	}
	AssetBrowserPopupDataFunction assetBrowserPopupDataFunction;
};

class EditorAssetRegistry
{
public:
	EditorAssetRegistry();
	~EditorAssetRegistry();
	
	
	static Vector<std::pair<std::string, std::string>> GetAssetBrowserPopupData(AssetType aAssetType, Volt::AssetHandle aAssetHandle);
private:
	void RegisterAssetBrowserPopupData(AssetType aAssetType, AssetBrowserPopupDataFunction aAssetBrowserPopupDataFunction);
    static std::unordered_map<AssetType, EditorAssetData> myAssetData;
};
