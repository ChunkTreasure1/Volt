#pragma once
#include <Volt/Asset/Asset.h>
#include <functional>

typedef std::function<std::vector<std::pair<std::string, std::string>>(Volt::AssetHandle)> AssetBrowserPopupDataFunction;
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
	
	
	static std::vector<std::pair<std::string, std::string>> GetAssetBrowserPopupData(Volt::AssetType aAssetType, Volt::AssetHandle aAssetHandle);
private:
	void RegisterAssetBrowserPopupData(Volt::AssetType aAssetType, AssetBrowserPopupDataFunction aAssetBrowserPopupDataFunction);
    static std::unordered_map<Volt::AssetType, EditorAssetData> myAssetData;
};