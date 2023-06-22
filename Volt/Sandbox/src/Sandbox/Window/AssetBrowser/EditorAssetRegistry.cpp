#include "sbpch.h"

#include "EditorAssetRegistry.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Material.h"
#define CREATE_ASSET_BROWSER_POPUP_DATA_FUNCTION(type, x) [](Volt::AssetHandle aAssetHandle)->std::vector<std::pair<std::string, std::string>>	\
			{																																	\
				auto asset = Volt::AssetManager::GetAsset<type>(aAssetHandle);																	\
				std::vector<std::pair<std::string, std::string>> data;																			\
				return data;																													\	
			}

std::unordered_map<Volt::AssetType, EditorAssetData> EditorAssetRegistry::myAssetData =
{
	{
		Volt::AssetType::Mesh,
		EditorAssetData(
			CREATE_ASSET_BROWSER_POPUP_DATA_FUNCTION(Volt::Mesh, apa))

	/*{std::make_pair("Submesh Count", std::to_string(asset->GetSubMeshes().size())))),std::make_pair("Material Path", asset->GetMaterial()->path.string()),std::make_pair("Vertex Count", std::to_string(asset->GetVertexCount())),std::make_pair("Index Count", std::to_string(asset->GetIndexCount()))}*/
	},
};

std::vector<std::pair<std::string, std::string>> EditorAssetRegistry::GetAssetBrowserPopupData(Volt::AssetType aAssetType, Volt::AssetHandle aAssetHandle)
{
	if (myAssetData.contains(aAssetType))
	{
		if (myAssetData[aAssetType].assetBrowserPopupDataFunction)
		{
			return myAssetData[aAssetType].assetBrowserPopupDataFunction(aAssetHandle);
		}
	}
	return std::vector<std::pair<std::string, std::string>>();
}

void EditorAssetRegistry::RegisterAssetBrowserPopupData(Volt::AssetType aAssetType, AssetBrowserPopupDataFunction aAssetBrowserPopupDataFunction)
{
	if (myAssetData[aAssetType].assetBrowserPopupDataFunction)
	{
		VT_CORE_WARN("Asset type {0} already has a popup data function registered, overwriting the function.", Volt::GetAssetTypeName(aAssetType));
	}
	myAssetData[aAssetType].assetBrowserPopupDataFunction = aAssetBrowserPopupDataFunction;
}
