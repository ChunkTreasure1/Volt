#include "sbpch.h"
#include "AssetIconLibrary.h"

#include <Volt/Asset/AssetManager.h>

void AssetIconLibrary::Initialize()
{
	myAssetIcons[Volt::AssetType::Material] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_material.dds");
	myAssetIcons[Volt::AssetType::Mesh] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_mesh.dds");
	myAssetIcons[Volt::AssetType::MeshSource] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_meshSource.dds");
	myAssetIcons[Volt::AssetType::Skeleton] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_skeleton.dds");
	myAssetIcons[Volt::AssetType::Animation] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_animation.dds");
	myAssetIcons[Volt::AssetType::AnimatedCharacter] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_animatedCharacter.dds");
	myAssetIcons[Volt::AssetType::Scene] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_scene.dds");
	myAssetIcons[Volt::AssetType::ParticlePreset] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_particlePreset.dds");
	myAssetIcons[Volt::AssetType::Prefab] = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_prefab.dds");
}

void AssetIconLibrary::Shutdowm()
{
	myAssetIcons.clear();
}

Ref<Volt::Texture2D> AssetIconLibrary::Get(Volt::AssetType type)
{
	if (!myAssetIcons.contains(type))
	{
		return nullptr;
	}

	return myAssetIcons.at(type);
}
