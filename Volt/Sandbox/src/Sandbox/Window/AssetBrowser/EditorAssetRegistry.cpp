#include "sbpch.h"

#include "EditorAssetRegistry.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/Scene/Scene.h"

#include "Volt/Physics/PhysicsMaterial.h"

#define ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandleVarName) [](Volt::AssetHandle aAssetHandleVarName)->Vector<std::pair<std::string, std::string>>



/*
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::Animation>(aAssetHandle);
				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("TEMPLATE", "TEMPLATE")),
				};
				return data;
			})

*/
std::unordered_map<Volt::AssetType, EditorAssetData> EditorAssetRegistry::myAssetData =
{
	{
		Volt::AssetType::Mesh,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::QueueAsset<Volt::Mesh>(aAssetHandle);
				if (!asset || !asset->IsValid())
				{
					return {};
				}

				std::filesystem::path sourceMeshPath = "Could not find the source mesh path";

				//const auto& dependencies = Volt::AssetManager::GetMetadataFromHandle(aAssetHandle).dependencies;

				//for (const auto& d : dependencies)
				//{
				//	if (d.extension().string() == ".fbx")
				//	{
				//		sourceMeshPath = d;
				//		break;
				//	}
				//}

				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Submesh Count", std::to_string(asset->GetSubMeshes().size())),
					
					std::make_pair("Vertex Count", Utility::ToStringWithThousandSeparator(asset->GetVertexCount())),
					std::make_pair("Index Count", Utility::ToStringWithThousandSeparator(asset->GetIndexCount())),
					std::make_pair("Source Mesh Path", sourceMeshPath.string())
				};
				return data;
			})
	},
	{
		Volt::AssetType::Animation,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::Animation>(aAssetHandle);
				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Duration", std::to_string(asset->GetDuration()) + " seconds"),
					std::make_pair("Frame Count", Utility::ToStringWithThousandSeparator(asset->GetFrameCount())),
					std::make_pair("Frames Per Second", std::to_string(asset->GetFramesPerSecond())),
				};
				return data;
			})
	},
	{
		Volt::AssetType::Skeleton,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::Skeleton>(aAssetHandle);
				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Joint Count", std::to_string(asset->GetJointCount()))
				};
				return data;
			})
	},
	{
		Volt::AssetType::Texture,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::QueueAsset<Volt::Texture2D>(aAssetHandle);
				if (!asset || !asset->IsValid())
				{
					return {};
				}

				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Width", std::to_string(asset->GetWidth())),
					std::make_pair("Height", std::to_string(asset->GetHeight())),
				};
				return data;
			})
	},
	{
		Volt::AssetType::Material,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::Material>(aAssetHandle);
				Vector<std::pair<std::string, std::string>> data =
				{
				};
				return data;
			})
	},
	{
		Volt::AssetType::Scene,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::Scene>(aAssetHandle);
				const auto& stats = asset->GetStatistics();
				const auto& layers = asset->GetLayers();
				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Entity Count", Utility::ToStringWithThousandSeparator(stats.entityCount)),
					std::make_pair("Layer Count", std::to_string(layers.size())),
				};
				for (int i = 0; i < layers.size(); ++i)
				{
				data.push_back(std::make_pair("Layer " + std::to_string(i), layers[i].name));

				}

				return data;
			})
	},
	{
		Volt::AssetType::AnimatedCharacter,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::AnimatedCharacter>(aAssetHandle);
				if (!asset->IsValid())
				{
					return Vector<std::pair<std::string, std::string>>();
				}
				const auto skeletonFilePath = Volt::AssetManager::GetFilePathFromAssetHandle(asset->GetSkeleton()->handle).string();
				const auto meshFilePath = Volt::AssetManager::GetFilePathFromAssetHandle(asset->GetSkin()->handle).string();

				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Skeleton Path", skeletonFilePath),
					std::make_pair("Mesh Path", meshFilePath),
					std::make_pair("Animation Count", std::to_string(asset->GetAnimationCount())),
				};
				return data;
			})
	},
	{
		Volt::AssetType::PhysicsMaterial,
		EditorAssetData(
			ASSET_BROWSER_POPUP_DATA_FUNCTION_IDENTIFIER(aAssetHandle)
			{
				auto asset = Volt::AssetManager::GetAsset<Volt::PhysicsMaterial>(aAssetHandle);
				Vector<std::pair<std::string, std::string>> data =
				{
					std::make_pair("Static Friction", std::to_string(asset->staticFriction)),
					std::make_pair("Dynamic Friction", std::to_string(asset->dynamicFriction)),
					std::make_pair("Bounciness", std::to_string(asset->bounciness)),
				};
				return data;
			})
	}
};

Vector<std::pair<std::string, std::string>> EditorAssetRegistry::GetAssetBrowserPopupData(Volt::AssetType aAssetType, Volt::AssetHandle aAssetHandle)
{
	if (myAssetData.contains(aAssetType))
	{
		if (myAssetData[aAssetType].assetBrowserPopupDataFunction)
		{
			return myAssetData[aAssetType].assetBrowserPopupDataFunction(aAssetHandle);
		}
	}
	return Vector<std::pair<std::string, std::string>>();
}

void EditorAssetRegistry::RegisterAssetBrowserPopupData(Volt::AssetType aAssetType, AssetBrowserPopupDataFunction aAssetBrowserPopupDataFunction)
{
	if (myAssetData[aAssetType].assetBrowserPopupDataFunction)
	{
		VT_LOG(LogVerbosity::Warning, "Asset type {0} already has a popup data function registered, overwriting the function.", Volt::GetAssetTypeName(aAssetType));
	}
	myAssetData[aAssetType].assetBrowserPopupDataFunction = aAssetBrowserPopupDataFunction;
}
