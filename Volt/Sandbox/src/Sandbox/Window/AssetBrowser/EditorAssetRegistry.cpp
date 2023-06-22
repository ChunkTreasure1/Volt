#include "sbpch.h"

#include "EditorAssetRegistry.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Material.h"

#define ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandleVarName) [](Volt::AssetHandle aAssetHandleVarName)->std::vector<std::pair<std::string, std::string>>

std::unordered_map<Volt::AssetType, EditorAssetData> EditorAssetRegistry::myAssetData =
{
	{
		Volt::AssetType::Mesh,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::Mesh>(aAssetHandle);
				std::vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Submesh Count", std::to_string(asset->GetSubMeshes().size())),
					std::make_pair("Material Path", asset->GetMaterial()->path.string()),
					std::make_pair("Vertex Count", std::to_string(asset->GetVertexCount())),
					std::make_pair("Index Count", std::to_string(asset->GetIndexCount()))
				};
				return data;
			})
	}
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
